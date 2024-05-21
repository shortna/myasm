#include "instructions.h"
#include "parser.h"
#include "tables.h"
#include "types.h"
#include <elf.h>
#include <string.h>
#include <stdio.h>

// Static opcodes
#define SOP_LOGICAL_IMM ((short)36)               // (0b00100100)
#define SOP_LOGICAL_SH_REG ((short)10)            // (0b00001010)
#define SOP_MOVE_WIDE ((short)37)                 // (0b00100101)
#define SOP_ADD_SUB_IMM ((short)34)               // (0b00100010)
#define SOP_PC_REl ((short)16)                    // (0b00010000)
#define SOP_EXCEPTIONS ((short)212)               // (0b11010100)
#define SOP_CONDITIONAL_BRANCH_IMM ((short)84)    // (0b01010100)
#define SOP_UNCONDITIONAL_BRANCH_IMM ((short)5)   // (0b00000101)
#define SOP_UNCONDITIONAL_BRANCH_REG ((short)107) // (0b01101011)
#define SOP_COMPARE_BRANCH ((short)26)            // (0b00011010)
#define SOP_TEST_BRANCH ((short)27)               // (0b00011011)
#define SOP_LDR_LITERAL ((short)24)               // (0b00011000)
#define SOP_LDR_STR_REG ((short)56)               // (0b00111000)
#define SOP_LDR_STR_IMM ((short)56)               // (0b00111000)
#define SOP_LDR_STR_UIMM ((short)57)              // (0b00111001)

#define BIT(p) (1ull << (p))
#define GENMASK(x) ((1ull << (x)) - 1ull)

#ifdef __clang__
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
// IMPORTANT
// instruction mnemonic MUST be in order of incresing opc field
static const Instruction INSTRUCTIONS[] = {
    {{"adr", "adrp"}, PCRELADDRESSING, {2, REGISTER, LABEL}},
    {{"b.", "bc."}, CONDITIONAL_BRANCH_IMM, {2, CONDITION, LABEL}},
    {{"b", "bl"}, UNCONDITIONAL_BRANCH_IMM, {1, LABEL}},
    {{"br", "blr", "ret"}, UNCONDITIONAL_BRANCH_REG, {1, REGISTER | OPTIONAL}},
    {{"cbz", "cbnz"}, COMPARE_BRANCH, {2, REGISTER, LABEL}},
    {{"tbz", "tbnz"}, TEST_BRANCH, {3, REGISTER, IMMEDIATE, LABEL}},
    {{"ldr", "ldrsw"}, LDR_LITERAL, {2, REGISTER, LABEL}},

    {{"movn", "movz", "movk"}, MOVEWIDE, {3, REGISTER, IMMEDIATE, SHIFT | OPTIONAL}},
    {{"add", "sub"}, ADDSUB_IMM, {4, REGISTER | SP, REGISTER | SP, IMMEDIATE, SHIFT | OPTIONAL}},
    {{"adds", "subs"}, ADDSUB_IMM, {4, REGISTER, REGISTER | SP, IMMEDIATE, SHIFT | OPTIONAL}},
    {{"and", "orr", "eor", "ands"}, LOGICAL_IMM, {3, REGISTER | SP, REGISTER, IMMEDIATE}},
    {{"and", "bic", "orr", "orn", "eor", "eon", "ands", "bics"}, LOGICAL_SH_REG, {4, REGISTER, REGISTER, REGISTER, SHIFT | OPTIONAL}},
    {{"svc", "hvc", "smc", "brk", "hlt", "tcancel"}, EXCEPTION, {1, IMMEDIATE}},
    {{"dcps1", "dcps2", "dcps3"}, EXCEPTION, {1, IMMEDIATE | OPTIONAL}},
    {{"strb", "ldrb", "ldrsb", "strh", "ldrh", "ldrsh", "str", "ldr", "ldrsw"}, LDR_STR_REG, {4, REGISTER, REGISTER | SP, REGISTER, EXTEND | SHIFT | OPTIONAL}},
    {{"strb", "ldrb", "ldrsb", "strh", "ldrh", "ldrsh", "str", "ldr", "ldrsw"}, LDR_STR_IMM, {3, REGISTER, REGISTER | SP, IMMEDIATE | OPTIONAL}},};

u8 searchMnemonic(const char *mnemonic) {
  for (u64 i = 0; i < sizeof(INSTRUCTIONS) / sizeof(*INSTRUCTIONS); i++) {
    u8 j = 0;
    while (INSTRUCTIONS[i].mnemonic[j]) {
      if (strcmp(mnemonic, INSTRUCTIONS[i].mnemonic[j]) == 0) {
        return 1;
      }
      j++;
    }
  }
  return 0;
}

u8 instructionIndex(InstructionType type, const char *mnemonic) {
  for (u8 i = 0; i < sizeof(INSTRUCTIONS) / sizeof(*INSTRUCTIONS); i++) {
    if (INSTRUCTIONS[i].type == type) {
      u8 j = 0;
      while (INSTRUCTIONS[i].mnemonic[j]) {
        if (strcmp(INSTRUCTIONS[i].mnemonic[j], mnemonic) == 0) {
          return j;
        }
        j++;
      }
    }
  }

  return 0;
}

// check if (size) bits of signed (n) fits in bounds
u8 checkBounds(i64 n, u8 size) {
  i64 mask = GENMASK(size - 1);
  if (n < 0) {
    return n > (~mask | 1);
  }

  return n < mask;
}

// WHAT THE FUCK
ArmInstruction assembleLdrStrImm(const Fields *instruction) {
  ArmInstruction assembled = 0;

  Register Rt, Rn;
  if (!parseRegister(instruction->fields[1].value, &Rt)) {
    // error here
    return 0;
  }

  if (!parseRegister(instruction->fields[3].value, &Rn)) {
    // error here
    return 0;
  }

  if (!Rn.extended) {
    // error here
    return 0;
  }

  u8 size = 0;
  const char instruction_postfix =
      instruction->fields[0].value[strlen(instruction->fields[0].value) - 1];

  u32 pimm_cap = 0;
  u8 scale = 1;

  switch (instruction_postfix) {
  case 'b':
    pimm_cap = 4095;
    size = 0;
    break;
  case 'h':
    pimm_cap = 8190;
    size = 1;
    scale = 2;
    break;
  case 'w':
    // ldrsw - accepts only 64 bit register
    if (!Rt.extended) {
      // error here
      return 0;
    }
    pimm_cap = 16380;
    size = 2;
    scale = 4;
    break;
  default:
    if (!Rt.extended) {
      size = 2;
      pimm_cap = 16380;
      scale = 4;
      break;
    }
    pimm_cap = 32760;
    size = 3;
    scale = 8;
    break;
  }

  i64 imm = 0;
  u8 imm_ind = 1;
  for (u8 i = 1; i < instruction->n_fields; i++) {
    if (instruction->fields[i].type == T_IMMEDIATE) {
      imm_ind = i;
    }
  }

  u8 op = SOP_LDR_STR_IMM;
  // unsigned offset
  if (instruction->fields[instruction->n_fields - 1].type == T_LSBRACE) {
    op = SOP_LDR_STR_UIMM;
    if (instruction->n_fields != 5) {
      if (!parseImmediateU32(instruction->fields[imm_ind].value, (u32 *)&imm)) {
        // error here
        return 0;
      }
      if (imm > pimm_cap || imm % scale != 0) {
        // error here
        return 0;
      }
    }
    imm = GENMASK(12) & (imm / scale);
  }
  // pre index
  // post index
  else {
    if (!parseImmediateI64(instruction->fields[imm_ind].value, &imm)) {
      // error here
      return 0;
    }
    if (imm < -256 || imm > 255) {
      // error here
      return 0;
    }
    imm = GENMASK(9) & imm; // theres some junk in, why?
    bool pre_indexed =
        instruction->fields[instruction->n_fields - 1].type == T_BANG;
    // if pre-indexed 3 else 1
    imm = GENMASK(12) & ((imm << 2) | (pre_indexed ? 3 : 1));
    if ((i32)imm < 0) {
      imm |= BIT(8);
    }
  }

  bool signed_ = instruction->fields->value[3] == 's';
  // "strb", "ldrb", "strh", "ldrh" - accepts as Rt only 32 bit register
  if (!signed_ && size < 2 && Rt.extended) {
    // error here
    return 0;
  }

  u8 opc = 0;
  if (*instruction->fields->value == 'l') {
    opc = 1;
  }
  if (signed_) {
    opc = 2 | (1 ^ Rt.extended);
  }

  // return 0 so makeAssemble do not write to file
  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)size << 30) | ((u32)op << 24) | ((u32)opc << 22) |
              ((u32)imm << 10) | ((u32)Rn.n << 5) | Rt.n;
  return assembled;
}

// WHAT THE FUCK v2
ArmInstruction assembleLdrStrReg(const Fields *instruction) {
  ArmInstruction assembled = 0;
  Register Rt, Rn, Rm;

  if (!parseRegister(instruction->fields[1].value, &Rt)) {
    // error here
    return 0;
  }

  if (!parseRegister(instruction->fields[3].value, &Rn)) {
    // error here
    return 0;
  }

  if (!parseRegister(instruction->fields[4].value, &Rm)) {
    // error here
    return 0;
  }

  if (!Rn.extended) {
    // error here
    return 0;
  }

  u8 size = 0;
  const char instruction_postfix =
      instruction->fields->value[strlen(instruction->fields->value) - 1];

  switch (instruction_postfix) {
  case 'b':
    size = 0;
    break;
  case 'h':
    size = 1;
    break;
  case 'w':
    if (!Rt.extended) {
      // error here
      return 0;
    }
    size = 2;
    break;
  default:
    size = 2 | Rt.extended; // 0b10 || 0b11
    break;
  }

  bool signed_ = instruction->fields->value[3] == 's';
  // "strb", "ldrb", "strh", "ldrh" - accepts as Rt only 32 bit register
  if (!signed_ && size < 2 && Rt.extended) {
    // error here
    return 0;
  }

  u8 opc = 1;
  if (*instruction->fields->value == 'l') {
    opc |= 2;
  }
  if (signed_) {
    opc |= 4 | (2 ^ Rt.extended);
  }

  u8 S = 2;
  TokenType arg;
  u8 option = 2 | Rm.extended;
  if ((arg = instruction->fields[5].type) != T_LSBRACE) {
    ExtendType ex;
    ShiftType sh;
    u8 imm = 0;
    if (arg == T_SHIFT) {
      if (!parseShift(instruction->fields[5].value, &sh)) {
        // error here
        return 0;
      }
      if (sh != SH_LSL) {
        // error here
        return 0;
      }
      if (size == 0) {
        if (!Rm.extended || instruction->fields[6].type != T_IMMEDIATE) {
          // error here
          return 0;
        }
      }
    } else if (arg == T_EXTEND) {
      if (!parseExtend(instruction->fields[5].value, &ex)) {
        // error here
        return 0;
      }
      switch (ex) {
      case UXTW:
      case SXTW:
        if (Rm.extended) {
          // error here
          return 0;
        }
        option = 2;
        if (ex == SXTW) {
          option = BIT(2) | option;
        }
        break;
      case SXTX:
        if (!Rm.extended) {
          // error here
          return 0;
        }
        option = 7;
        break;
      default:
        // error here
        return 0;
      }
    }
    if (instruction->fields[6].type == T_IMMEDIATE) {
      if (!parseImmediateU8(instruction->fields[6].value, &imm)) {
        // error here
        return 0;
      }
      if (imm != size && imm != 0) {
        // error here
        return 0;
      }
      if (imm == size) {
        S = BIT(2) | S;
      }
    }
  }

  // return 0 so makeAssemble do not write to file
  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)size << 30) | ((u32)SOP_LDR_STR_REG << 24) |
              ((u32)opc << 21) | ((u32)Rm.n << 16) | ((u32)option << 13) |
              ((u32)S << 10) | ((u32)Rn.n << 5) | Rt.n;
  return assembled;
}

ArmInstruction assembleLdrLiteral(const Fields *instruction) {
  ArmInstruction assembled = 0;
  Register Rt = {0};
  parseRegister(instruction->fields[1].value, &Rt);

  u8 opc = instructionIndex(LDR_LITERAL, instruction->fields->value) + 1;
  if (!Rt.extended && opc == 2) {
    errorFields("Instruction supports only 64 bits registers", instruction);
  }
  if (!Rt.extended && opc == 1) {
    opc = 0;
  }

  i64 label_pc = getLabelPc(instruction->fields[2].value);
  if (label_pc == -1) {
    errorFields("Argument 2 - unknown label", instruction);
  }

  const u8 offset_size = 19;
  i32 offset = (label_pc - CONTEXT.pc) / ARM_INSTRUCTION_SIZE;
  if (!checkBounds(offset, offset_size)) {
    errorFields("Label is out of bounds", instruction);
  }
  offset = GENMASK(offset_size) & offset;
  if (offset < 0) {
    offset |= BIT(offset_size - 1);
  }

  // return 0 so makeAssemble do not write to file
  if (ERRORS) {
    return 0;
  }
  assembled =
      ((u32)opc << 30) | ((u32)SOP_LDR_LITERAL << 24) | (offset << 5) | Rt.n;
  return assembled;
}

ArmInstruction assembleUnconditionalBranchReg(const Fields *instruction) {
  ArmInstruction assembled = 0;
  Register Rn = {true, 30};

  u8 op =
      instructionIndex(UNCONDITIONAL_BRANCH_REG, instruction->fields[0].value);
  if (instruction->n_fields == 1 && op != 2) {
    errorFields("Not enough arguments", instruction);
  }

  if (instruction->n_fields != 1) {
    parseRegister(instruction->fields[1].value, &Rn);

    if (!Rn.extended) {
      errorFields("Registers must have size of 64bits", instruction);
    }
  }

  const u8 Z = 0;
  const u16 op2 = 0xf80;

  // return 0 so makeAssemble do not write to file
  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)SOP_UNCONDITIONAL_BRANCH_REG << 25) | ((u32)Z << 24) |
              ((u32)op << 21) | ((u32)op2 << 9) | ((u32)Rn.n << 5) | 0;
  return assembled;
}

ArmInstruction assembleCompareBranch(const Fields *instruction) {
  ArmInstruction assembled = 0;
  Register Rt;

  parseRegister(instruction->fields[1].value, &Rt);
  bool sf = Rt.extended;

  i64 label_pc = getLabelPc(instruction->fields[2].value);
  if (label_pc == -1) {
    errorFields("Argument 2 - unknown label", instruction);
  }

  const u8 offset_size = 19;
  i32 offset = (label_pc - CONTEXT.pc) / ARM_INSTRUCTION_SIZE;
  if (!checkBounds(offset, offset_size)) {
    errorFields("Label is out of bounds", instruction);
  }
  offset = GENMASK(offset_size) & offset;
  if (offset < 0) {
    offset |= BIT(offset_size - 1);
  }

  u8 op = instructionIndex(COMPARE_BRANCH, instruction->fields[0].value);

  // return 0 so makeAssemble do not write to file
  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)sf << 31) | ((u32)SOP_COMPARE_BRANCH << 25) |
              ((u32)op << 24) | ((u32)offset << 5) | Rt.n;
  return assembled;
}

ArmInstruction assembleTestBranch(const Fields *instruction) {
  ArmInstruction assembled = 0;
  Register Rt;

  parseRegister(instruction->fields[1].value, &Rt);

  u8 imm;
  if (!parseImmediateU8(instruction->fields[2].value, &imm)) {
    errorFields("Argument 2 must be x < register size && x >= 0", instruction);
  }

  bool sf = Rt.extended;
  if (imm > 31 && !sf) {
    errorFields("Argument 2 must be x < register size && x >= 0", instruction);
  }
  imm = imm & 0x1f;

  i64 label_pc = getLabelPc(instruction->fields[3].value);
  if (label_pc == -1) {
    errorFields("Argument 3 - unknown label", instruction);
  }

  const u8 offset_size = 14;
  i32 offset = (label_pc - CONTEXT.pc) / ARM_INSTRUCTION_SIZE;
  if (!checkBounds(offset, offset_size)) {
    errorFields("Label is out of bounds", instruction);
  }
  offset = GENMASK(offset_size) & offset;
  if (offset < 0) {
    offset |= BIT(offset_size - 1);
  }

  u8 op = instructionIndex(TEST_BRANCH, instruction->fields[0].value);

  // return 0 so makeAssemble do not write to file
  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)sf << 31) | ((u32)SOP_TEST_BRANCH << 25) | ((u32)op << 24) |
              ((u32)imm << 19) | ((u32)offset << 5) | Rt.n;
  return assembled;
}

ArmInstruction assembleConditionalBranchImm(const Fields *instruction) {
  ArmInstruction assembled = 0;
  ConditionType c = 0;
  parseCondition(instruction->fields[1].value, &c);

  i64 label_pc = getLabelPc(instruction->fields[2].value);
  if (label_pc == -1) {
    errorFields("Argument 2 - unknown label", instruction);
  }

  const u8 offset_size = 19;
  i32 offset = (label_pc - CONTEXT.pc) / ARM_INSTRUCTION_SIZE;
  if (!checkBounds(offset, offset_size)) {
    errorFields("Label is out of bounds", instruction);
  }
  offset = GENMASK(offset_size) & offset;
  if (offset < 0) {
    offset |= BIT(offset_size - 1);
  }

  u8 o = instructionIndex(CONDITIONAL_BRANCH_IMM, instruction->fields[0].value);

  // return 0 so makeAssemble do not write to file
  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)SOP_CONDITIONAL_BRANCH_IMM << 24) | ((u32)offset << 5) |
              (o << 4) | c;
  return assembled;
}

ArmInstruction assembleUnconditionalBranchImm(const Fields *instruction) {
  ArmInstruction assembled = 0;

  i64 label_pc = getLabelPc(instruction->fields[1].value);
  if (label_pc == -1) {
    errorFields("Argument 1 - unknown label", instruction);
  }

  const u8 offset_size = 26;
  i32 offset = (label_pc - CONTEXT.pc) / ARM_INSTRUCTION_SIZE;
  if (!checkBounds(offset, offset_size)) {
    errorFields("Label is out of bounds", instruction);
  }

  offset = GENMASK(offset_size) & offset;
  if (offset < 0) {
    offset |= BIT(offset_size - 1);
  }

  u8 op =
      instructionIndex(UNCONDITIONAL_BRANCH_IMM, instruction->fields[0].value);
  assembled =
      ((u32)op << 31) | ((u32)SOP_UNCONDITIONAL_BRANCH_IMM << 26) | offset;
  return assembled;
}

ArmInstruction assemblePcRelAddressing(const Fields *instruction) {
  ArmInstruction assembled = 0;
  Register Rd;

  parseRegister(instruction->fields[1].value, &Rd);

  if (!Rd.extended) {
    errorFields("Registers must have size of 64bits", instruction);
  }

  i64 label_pc = getLabelPc(instruction->fields[2].value);
  if (label_pc == -1) {
    errorFields("Argument 2 - unknown label", instruction);
  }

  const u8 offset_size = 21;
  i32 offset = (label_pc - CONTEXT.pc) / ARM_INSTRUCTION_SIZE;
  if (!checkBounds(offset, offset_size)) {
    errorFields("Label is out of bounds", instruction);
  }

  offset = GENMASK(offset_size) & offset;
  if (offset < 0) {
    offset |= BIT(offset_size - 1);
  }

  u8 op = instructionIndex(PCRELADDRESSING, instruction->fields->value);
  u8 immlo = offset & 3; // low 2 bits
  u32 immhi = offset & (u32)-4; // remove low 2 bits

  // return 0 so makeAssemble do not write to file
  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)op << 31) | ((u32)immlo << 29) | ((u32)SOP_PC_REl << 24) |
              ((u32)immhi << 5) | Rd.n;
  return assembled;
}

// SHAMELESSLY STOLEN FROM LLVM

// Is this number's binary representation all 1s?
u8 isMask(u64 imm) { return ((imm + 1) & imm) == 0; }

// Is this number's binary representation one or more 1s followed by
// one or more 0s?
u8 isShiftedMask(u64 imm) { return isMask((imm - 1) | imm); }

#define countTrailingZeros(x) __builtin_ctzll(x)
#define countTrailingOnes(x) __builtin_ctzll(~x)
#define countLeadingOnes(x) __builtin_clzll(~x)

bool encodeBitmaskImmediate(u64 imm, u64 *encoding, bool extended) {
  if (imm == 0ULL || imm == ~0ULL)
    return false;

  // First, determine the element size.
  u32 size = extended ? 64u : 32u;

  do {
    size >>= 1;
    u64 mask = (1ULL << size) - 1;

    if ((imm & mask) != ((imm >> size) & mask)) {
      size <<= 1;
      break;
    }
  } while (size > 2);

  // Second, determine the rotation to make the element be: 0^m 1^n.
  u32 cto, ctz;
  u64 mask = ((u64)-1LL) >> (64 - size);
  imm &= mask;

  if (isShiftedMask(imm)) {
    ctz = countTrailingZeros(imm);
    cto = countTrailingOnes(imm >> ctz);
  } else {
    imm |= ~mask;
    if (!isShiftedMask(~imm))
      return false;

    u8 clo = countLeadingOnes(imm);
    ctz = 64 - clo;
    cto = clo + countTrailingOnes(imm) - (64 - size);
  }

  // Encode in Immr the number of RORs it would take to get *from* 0^m 1^n
  // to our target value, where I is the number of RORs to go the opposite
  // direction.
  u32 immr = (size - ctz) & (size - 1);

  // If size has a 1 in the n'th bit, create a value that has zeroes in
  // bits [0, n] and ones above that.
  u64 nimms = ~(size - 1) << 1;

  // Or the CTO value into the low bits, which must be below the Nth bit
  // bit mentioned above.
  nimms |= (cto - 1);

  // Extract the seventh bit and toggle it to create the N field.
  u8 N = ((nimms >> 6) & 1) ^ 1;

  *encoding = (N << 12) | (immr << 6) | (nimms & 0x3f);
  return true;
}

ArmInstruction assembleLogicalImm(const Fields *instruction) {
  ArmInstruction assembled = 0;

  Register Rd;
  Register Rn;

  parseRegister(instruction->fields[1].value, &Rd);
  parseRegister(instruction->fields[2].value, &Rn);

  bool sf = 0;
  if (Rd.extended && Rn.extended) {
    sf = 1;
  } else if (!Rd.extended && !Rn.extended) {
    sf = 0;
  } else {
    errorFields("Registers must have same size", instruction);
  }

  u64 imm = 0;
  parseImmediateU64(instruction->fields[3].value, &imm);

  u64 encoding;
  if (!encodeBitmaskImmediate(imm, &encoding, sf)) {
    errorFields("Argument 3 must be immediate a 32-bit or 64-bit pattern of "
                "identical elements of size = 2, 4, 8, 16, 32, or 64 bits",
                instruction);
  }

  u8 opc = instructionIndex(LOGICAL_IMM, instruction->fields[0].value);

  // return 0 so makeAssemble do not write to file
  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)sf << 31) | ((u32)opc << 29) |
              ((u32)SOP_LOGICAL_IMM << 23) | (encoding << 10) |
              ((u32)Rn.n << 5) | Rd.n;
  return assembled;
}

ArmInstruction assembleLogicalShReg(const Fields *instruction) {
  ArmInstruction assembled = 0;

  Register Rd;
  Register Rn;
  Register Rm;

  parseRegister(instruction->fields[1].value, &Rd);
  parseRegister(instruction->fields[2].value, &Rn);
  parseRegister(instruction->fields[3].value, &Rm);

  bool sf = 0;
  if (Rd.extended && Rn.extended && Rm.extended) {
    sf = 1;
  } else if (!Rd.extended && !Rn.extended && !Rm.extended) {
    sf = 0;
  } else {
    errorFields("Registers must have same size", instruction);
  }

  ShiftType t = SH_LSL;
  u8 sh_imm = 0;
  if (instruction->n_fields == 6) {
    parseShift(instruction->fields[4].value, &t);
    if (!parseImmediateU8(instruction->fields[5].value, &sh_imm)) {
      errorFields("Argument 5 must be x < register size && x >= 0", instruction);
    }
    if (sh_imm >= (sf ? 64 : 32)) {
      errorFields("Argument 5 must be x < register size && x >= 0", instruction);
    }
  }

  // opc  N  j  jb
  //  00  0  0  000
  //  00  1  1  001
  //  01  0  2  010
  //  01  1  3  011
  //  10  0  4  100
  //  10  1  5  101
  //  11  0  6  110
  //  11  1  7  111

  u8 opc = instructionIndex(LOGICAL_SH_REG, instruction->fields[0].value);
  u8 N = opc & 1;
  opc = opc >> 1;

  // return 0 so makeAssemble do not write to file
  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)sf << 31) | ((u32)opc << 29) |
              ((u32)SOP_LOGICAL_SH_REG << 24) | ((u32)t << 22) |
              ((u32)N << 21) | ((u32)Rm.n << 16) | ((u32)sh_imm << 10) |
              ((u32)Rn.n << 5) | Rd.n;

  return assembled;
}

ArmInstruction assembleMoveWide(const Fields *instruction) {
  ArmInstruction assembled = 0;
  Register Rd;
  parseRegister(instruction->fields[1].value, &Rd);

  u16 imm = 0;
  if (!parseImmediateU16(instruction->fields[2].value, &imm)) {
    errorFields("Argument 2 must have type uint16", instruction);
  }

  bool sf = Rd.extended;

  ShiftType t = SH_LSL;
  u8 sh_imm = 0;
  if (instruction->n_fields > 3) {
    parseShift(instruction->fields[3].value, &t);

    if (t != SH_LSL) {
      errorFields("Only 'LSL' shift allowed", instruction);
    }

    if (!parseImmediateU8(instruction->fields[4].value, &sh_imm)) {
      errorFields("Argument 4 must have type uint8", instruction);
    }

    if (sf && (sh_imm > 48 || sh_imm % 16 != 0)) {
      errorFields("Argument 4 must be multiple of 16 and be less than 48", instruction);
    } else if (!sf && sh_imm != 0 && sh_imm != 16) {
      errorFields("Argument 4 must be 16 or 0", instruction);
    }
  }

  u8 opc = instructionIndex(MOVEWIDE, instruction->fields->value);
  if (opc != 0) { // because opc 0b01 not allocated
    opc += 1;
  }

  u8 hw = sh_imm / 16;

  // return 0 so makeAssemble do not write to file
  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)sf << 31) | ((u32)opc << 29) | ((u32)SOP_MOVE_WIDE << 23) |
              ((u32)hw << 21) | ((u32)imm << 5) | Rd.n;
  return assembled;
}

ArmInstruction assembleAddSubImm(const Fields *instruction) {
  ArmInstruction assembled = 0;

  Register Rd;
  Register Rn;

  parseRegister(instruction->fields[1].value, &Rd);
  parseRegister(instruction->fields[2].value, &Rn);

  bool sf = 0;
  if (Rd.extended && Rn.extended) {
    sf = 1;
  } else if (!Rd.extended && !Rn.extended) {
    sf = 0;
  } else {
    errorFields("Registers must have same size", instruction);
  }

  u16 imm;
  if (!parseImmediateU16(instruction->fields[3].value, &imm)) {
    errorFields("Argument 3 must be x <= 4095 && x >= 0", instruction);
  }

  if (imm > 4095) {
    errorFields("Argument 3 must be x <= 4095 && x >= 0", instruction);
  }

  ShiftType t = SH_LSL;
  u8 sh_imm = 0;
  if (instruction->n_fields == 6) {
    parseShift(instruction->fields[4].value, &t);

    if (t != SH_LSL) {
      errorFields("Only 'LSL 12' or 'LSL 0' allowed", instruction);
    }

    if (!parseImmediateU8(instruction->fields[5].value, &sh_imm)) {
      errorFields("Argument 5 must be equal to 12 or 0", instruction);
    }

    if (sh_imm != 12 && sh_imm != 0) {
      errorFields("Argument 5 must be equal to 12 or 0", instruction);
    }
  }

  u8 op = instructionIndex(ADDSUB_IMM, instruction->fields[0].value);
  u8 S = *(instruction->fields->value + 3) != '\0';

  bool sh = sh_imm == 12;

  // return 0 so makeAssemble do not write to file
  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)sf << 31) | ((u32)op << 30) | ((u32)S << 29) |
              ((u32)SOP_ADD_SUB_IMM << 23) | ((u32)sh << 22) |
              ((u32)imm << 10) | ((u32)Rn.n << 5) | Rd.n;

  return assembled;
}

ArmInstruction assembleException(const Fields *instruction) {
  ArmInstruction assembled = 0;

  u8 LL = instructionIndex(EXCEPTION, instruction->fields[0].value) + 1;
  u8 opc = 5; // 0b101

  u16 imm = 0;
  if (instruction->n_fields > 1) {
    if (!parseImmediateU16(instruction->fields[1].value, &imm)) {
      errorFields("Argument 1 must have type uint16", instruction);
    }

    // opc  LL  mnemonic
    // 101  01  DCPS1
    // 101  10  DCPS2
    // 101  11  DCPS3
    // do not process DCPS* instructions since the already set
    if (*instruction->fields[0].value != 'd') {
      // opc  LL  mnemonic
      // 000  01  SVC
      // 000  10  HVC
      // 000  11  SMC
      // LL already set
      opc = 0;
      if (LL - 1 > 2) {
        // opc  LL  0bLL  0bLL - 1  (0bLL - 1) >> 1  mnemonic
        // 001  4   0100  0011      0001             BRK
        // 010  5   0101  0100      0010             HLT
        // 010  6   0110  0101      0010             TCANCEL
        opc = (LL - 1) >> 1;
        LL = 0;
      }
    }
  }

  // return 0 so makeAssemble do not write to file
  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)SOP_EXCEPTIONS << 24u) | ((u32)opc << 21u) |
              ((u32)imm << 5u) | (0 << 2u) | LL;

  return assembled;
}

u8 compareSignatures(const Signature *s1, const Signature *s2) {
  u8 i = 0;
  Argument *s1_args = ((Argument *)s1) + 1;
  Argument *s2_args = ((Argument *)s2) + 1;
  while (i < s1->n_args) {
    // if args differs from what expected and optional not set
    if ((s1_args[i] & s2_args[i]) == 0 && !(s1_args[i] & OPTIONAL)) {
      return 0;
    }
    i++;
  }

  return 1;
}

InstructionType getInstructionType(const char *mnemonic, Signature *s) {
  for (u64 i = 0; i < sizeof(INSTRUCTIONS) / sizeof(*INSTRUCTIONS); i++) {
    if (compareSignatures(&INSTRUCTIONS[i].s, s)) {
      u8 j = 0;
      while (INSTRUCTIONS[i].mnemonic[j]) {
        if (strcmp(INSTRUCTIONS[i].mnemonic[j], mnemonic) == 0) {
          return INSTRUCTIONS[i].type;
        }
        j++;
      }
    }
  }
  return NONE;
}

Signature decodeTokens(const Fields *instruction) {
  Signature s = {0};
  Argument *args = ((Argument *)&s);
  u8 i = 1, n = 1;

  while (i < instruction->n_fields) {
    switch (instruction->fields[i].type) {
    case T_REGISTER: {
      Register r;
      parseRegister(instruction->fields[i].value, &r);
      if (r.n == REGISTER_ZR_SP) {
        if (strcmp(instruction->fields[i].value + 1, "zr") != 0) {
          args[n] = SP;
          break;
        }
      }
      args[n] = REGISTER;
      break;
    }
    case T_LABEL:
      args[n] = LABEL;
      break;
    case T_IMMEDIATE:
      args[n] = IMMEDIATE;
      break;
    case T_SHIFT:
      if (i + 1 < instruction->n_fields &&
          instruction->fields[i + 1].type == T_IMMEDIATE) {
        args[n] = SHIFT;
        break;
      }
      errorFields("Shift without parameter", instruction);
      break;
    case T_EXTEND:
      args[n] = EXTEND;
      break;
    case T_CONDITION:
      args[n] = CONDITION;
      break;
    case T_RSBRACE:
    case T_LSBRACE:
    case T_BANG:
    case T_EQUAL:
      n--;
      break;
    default:
      NULL;
    }
    i++;
    n++;
  }

  s.n_args = n;
  return s;
}

ArmInstruction assemble(const Fields *instruction) {
  Signature s = decodeTokens(instruction);
  InstructionType it = getInstructionType(instruction->fields->value, &s);

  ArmInstruction i = 0;
  switch (it) {
  case LOGICAL_IMM:
    i = assembleLogicalImm(instruction);
    break;
  case LOGICAL_SH_REG:
    i = assembleLogicalShReg(instruction);
    break;
  case MOVEWIDE:
    i = assembleMoveWide(instruction);
    break;
  case ADDSUB_IMM:
    i = assembleAddSubImm(instruction);
    break;
  case UNCONDITIONAL_BRANCH_REG:
    i = assembleUnconditionalBranchReg(instruction);
    break;
  case EXCEPTION:
    i = assembleException(instruction);
    break;
  case LDR_STR_REG:
    i = assembleLdrStrReg(instruction);
    break;
  case LDR_STR_IMM:
    i = assembleLdrStrImm(instruction);
    break;
  case PCRELADDRESSING:
    i = assemblePcRelAddressing(instruction);
    break;
  case CONDITIONAL_BRANCH_IMM:
    i = assembleConditionalBranchImm(instruction);
    break;
  case UNCONDITIONAL_BRANCH_IMM:
    i = assembleUnconditionalBranchImm(instruction);
    break;
  case COMPARE_BRANCH:
    i = assembleCompareBranch(instruction);
    break;
  case TEST_BRANCH:
    i = assembleTestBranch(instruction);
    break;
  case LDR_LITERAL:
    i = assembleLdrLiteral(instruction);
    break;
  case NONE:
    if (searchMnemonic(instruction->fields->value)) {
      errorFields("Incorrect arguments for instruction", instruction);
    } else {
      errorFields("Unknown instruction", instruction);
    }
    return 0;
  }

  return i ? i : 0;
}
