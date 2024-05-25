#include "instructions.h"
#include "parser.h"
#include "tables.h"
#include "types.h"
#include <elf.h>
#include <string.h>

// Static opcodes
#define SOP_LOGICAL_IMM (36)               // (0b00100100)
#define SOP_LOGICAL_SH_REG (10)            // (0b00001010)
#define SOP_MOVE_WIDE (37)                 // (0b00100101)
#define SOP_ADD_SUB_IMM (34)               // (0b00100010)
#define SOP_PC_REl (16)                    // (0b00010000)
#define SOP_EXCEPTIONS (212)               // (0b11010100)
#define SOP_CONDITIONAL_BRANCH_IMM (84)    // (0b01010100)
#define SOP_UNCONDITIONAL_BRANCH_IMM (5)   // (0b00000101)
#define SOP_UNCONDITIONAL_BRANCH_REG (107) // (0b01101011)
#define SOP_COMPARE_BRANCH (26)            // (0b00011010)
#define SOP_TEST_BRANCH (27)               // (0b00011011)
#define SOP_LDR_LITERAL (24)               // (0b00011000)
#define SOP_LDR_STR_REG (56)               // (0b00111000)
#define SOP_LDR_STR_IMM (56)               // (0b00111000)
#define SOP_LDR_STR_UIMM (57)              // (0b00111001)
#define SOP_CONDITIONAL_COMPARE_REG (210)  // (0b11010010)
#define SOP_CONDITIONAL_COMPARE_IMM (210)  // (0b11010010)

#define GENMASK(x) ((1ull << (x)) - 1ull)

#define OPTIONAL BIT(31)
// RSP and RZR in enum are different but in practice both encoded as 31
#define REG_SP (31)

typedef struct Signature {
  TokenType args[FIELDS_SIZE - 1]; // -1 since first args is mnemonic
} Signature;

typedef enum InstructionType {
  NONE,
  LOGICAL_IMM,
  LOGICAL_SH_REG,
  MOVEWIDE,
  ADDSUB_IMM,
  PCRELADDRESSING,
  EXCEPTION,
  CONDITIONAL_BRANCH_IMM,
  UNCONDITIONAL_BRANCH_IMM,
  UNCONDITIONAL_BRANCH_REG,
  COMPARE_BRANCH,
  TEST_BRANCH,
  LDR_LITERAL,
  LDR_STR_REG,
  LDR_STR_IMM_POST_INDEX,
  LDR_STR_IMM_PRE_INDEX,
  LDR_STR_IMM_UNSIGNDE_OFFSET,
  CONDITIONAL_COMPARE_REG,
  CONDITIONAL_COMPARE_IMM,
} InstructionType;

typedef struct Instruction {
  char const *const mnemonic[10];
  const InstructionType type;
  const Signature s;
} Instruction;

// IMPORTANT
// instruction mnemonic MUST be in order of incresing opc field
static const Instruction INSTRUCTIONS[] = {
    {{"b."}, CONDITIONAL_BRANCH_IMM, {T_CONDITION, T_LABEL, T_EOL}},
    {{"b", "bl"}, UNCONDITIONAL_BRANCH_IMM, {T_LABEL, T_EOL}},
    {{"cbz", "cbnz"}, COMPARE_BRANCH, {T_REGISTER, T_LABEL, T_EOL}},
    {{"tbz", "tbnz"}, TEST_BRANCH, {T_REGISTER, T_IMMEDIATE, T_LABEL, T_EOL}},
    {{"br", "blr", "ret"}, UNCONDITIONAL_BRANCH_REG, {T_REGISTER | OPTIONAL, T_EOL}},

    {{"adr", "adrp"}, PCRELADDRESSING, {T_REGISTER, T_LABEL, T_EOL}},
    {{"movn", "movz", "movk"}, MOVEWIDE, {T_REGISTER, T_IMMEDIATE, T_SHIFT | OPTIONAL, T_IMMEDIATE | OPTIONAL, T_EOL}},
    {{"add", "sub", "adds", "subs"}, ADDSUB_IMM, {T_REGISTER, T_REGISTER, T_IMMEDIATE, T_SHIFT | OPTIONAL, T_IMMEDIATE | OPTIONAL, T_EOL}},
    {{"and", "bic", "orr", "orn", "eor", "eon", "ands", "bics"}, LOGICAL_SH_REG, {T_REGISTER, T_REGISTER, T_REGISTER, T_SHIFT | OPTIONAL, T_IMMEDIATE | OPTIONAL, T_EOL}},
    {{"svc", "hvc", "smc", "brk", "hlt", "tcancel", "dcps1", "dcps2", "dcps3"}, EXCEPTION, {T_IMMEDIATE, T_EOL}},
    {{"and", "orr", "eor", "ands"}, LOGICAL_IMM, {T_REGISTER, T_REGISTER, T_IMMEDIATE, T_EOL}},

    {{"ldr", "ldrsw"}, LDR_LITERAL, {T_REGISTER, T_LABEL, T_EOL}},
    {{"strb", "ldrb", "ldrsb", "strh", "ldrh", "ldrsh", "str", "ldr", "ldrsw"}, LDR_STR_REG, {T_REGISTER, T_RSBRACE, T_REGISTER, T_REGISTER, T_EXTEND | T_SHIFT | OPTIONAL, T_IMMEDIATE | OPTIONAL, T_LSBRACE, T_EOL}},
    {{"strb", "ldrb", "ldrsb", "strh", "ldrh", "ldrsh", "str", "ldr", "ldrsw"}, LDR_STR_IMM_POST_INDEX,      {T_REGISTER, T_RSBRACE, T_REGISTER, T_LSBRACE, T_IMMEDIATE, T_EOL}},
    {{"strb", "ldrb", "ldrsb", "strh", "ldrh", "ldrsh", "str", "ldr", "ldrsw"}, LDR_STR_IMM_PRE_INDEX,       {T_REGISTER, T_RSBRACE, T_REGISTER, T_IMMEDIATE, T_LSBRACE, T_BANG, T_EOL}},
    {{"strb", "ldrb", "ldrsb", "strh", "ldrh", "ldrsh", "str", "ldr", "ldrsw"}, LDR_STR_IMM_UNSIGNDE_OFFSET, {T_REGISTER, T_RSBRACE, T_REGISTER, T_LSBRACE | T_IMMEDIATE, T_LSBRACE | OPTIONAL, T_EOL}},

    {{"ccmn", "ccmp"}, CONDITIONAL_COMPARE_REG, {T_REGISTER, T_REGISTER, T_IMMEDIATE, T_CONDITION, T_EOL}},
    {{"ccmn", "ccmp"}, CONDITIONAL_COMPARE_IMM, {T_REGISTER, T_IMMEDIATE, T_IMMEDIATE, T_CONDITION, T_EOL}},
};

i8 searchMnemonic(const char *mnemonic) {
  for (u64 i = 0; i < sizeof(INSTRUCTIONS) / sizeof(*INSTRUCTIONS); i++) {
    u8 j = 0;
    while (INSTRUCTIONS[i].mnemonic[j]) {
      if (strcmp(mnemonic, INSTRUCTIONS[i].mnemonic[j]) == 0) {
        return i;
      }
      j++;
    }
  }
  return -1;
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

u8 compareSignatures(const Signature *s1, const Signature *s2) {
  u8 i = 0;
  u8 j = 0;
  while (s1->args[i] != T_EOL) {
    // if args differs from what expected and optional not set
    if (!(s1->args[i] & s2->args[j])) {
      if (!(s1->args[i] & OPTIONAL)) {
        return 0;
      }
      j--;
    }
    j++;
    i++;
  }
  if (s2->args[j] != T_EOL) {
    return 0;
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

Signature getSignature(const Fields *instruction) {
  Signature s = {0};
  for (u8 i = 1; i < instruction->n_fields; i++) {
    s.args[i - 1] = instruction->fields[i].type;
  }
  return s;
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

/*************************/
/* INSTRUCINONS ENCODING */
/*************************/

// WHAT THE FUCK
ArmInstruction assembleLdrStrImm(const Fields *instruction, InstructionType type) {
  ArmInstruction assembled = 0;

  Register Rt, Rn;
  if (!parseRegister(instruction->fields[1].value, &Rt)) {
    errorFields("Argument 1 must be register R0 - R29 or RZR/LR", instruction);
  }

  if (Rt.reg == RSP) {
    errorFields("Argument 1 must be register R0 - R29 or RZR/LR", instruction);
  }

  if (!parseRegister(instruction->fields[3].value, &Rn)) {
    errorFields("Argument 2 must be any extended register", instruction);
  }

  if (Rn.reg == RSP) {
    Rn.reg = REG_SP; 
  }

  if (!Rn.extended) {
    errorFields("Argument 2 must have size of 64bits", instruction);
  }

  u8 size = 0;
  const char instruction_postfix =
      instruction->fields->value[strlen(instruction->fields->value) - 1];

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
      errorFields("ldrsw - accepts as Argument 1 only 64 bit register",
                  instruction);
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
  u8 imm_ind = 0;
  u8 op = SOP_LDR_STR_IMM;

  if (type == LDR_STR_IMM_UNSIGNDE_OFFSET) {
    if (instruction->fields[4].type == T_LSBRACE &&
        instruction->fields[5].type != T_EOL) {
      errorFields("Incorrect arguments for instruction", instruction);
    }
    if (instruction->fields[4].type == T_IMMEDIATE &&
        instruction->fields[5].type != T_LSBRACE) {
      errorFields("Incorrect arguments for instruction", instruction);
    }

    op = SOP_LDR_STR_UIMM;
    if (instruction->fields[4].type == T_IMMEDIATE) {
      if (!parseImmediateU32(instruction->fields[4].value, (u32 *)&imm)) {
        errorFields("Argument 4 must have type of uint32", instruction);
      }
      if (imm > pimm_cap || imm % scale != 0) {
        errorFields(
            "Immediate must be a multiple of 4 in the range 0 to 4095 "
            "for 8bits, 0 to 8190 for 16bits, 0 to 16380 for 32bits and "
            "in range 0 to 32760 for 64bits",
            instruction);
      }
      imm = GENMASK(12) & (imm / scale);
    }
  } else {
    bool pre_indexed = false;
    if (type == LDR_STR_IMM_PRE_INDEX) {
      pre_indexed = true;
      imm_ind = 4;
    } else {
      imm_ind = 5;
    }

    if (!parseImmediateI64(instruction->fields[imm_ind].value, &imm)) {
      errorFields("Failed to parse immediate", instruction);
    }

    if (imm < -256 || imm > 255) {
      errorFields("Immediate must be in range -256 to 255", instruction);
    }
    imm = GENMASK(9) & imm; // theres some junk in, why?
                            
    // if pre-indexed 3 else 1
    imm = GENMASK(12) & ((imm << 2) | (pre_indexed ? 3 : 1));
    if ((i32)imm < 0) {
      imm |= BIT(8);
    }
  }

  bool signed_ = instruction->fields->value[strlen(instruction->fields->value) - 1] == 's';
  // "strb", "ldrb", "strh", "ldrh" - accepts as Rt only 32 bit register
  if (!signed_ && size < 2 && Rt.extended) {
    errorFields(
        "Instruction accepts as argument 1 only 32 bit register",
        instruction);
  }

  u8 opc = 0;
  if (*instruction->fields->value == 'l') {
    opc = 1;
  }
  if (signed_) {
    opc = 2 | (1 ^ Rt.extended);
  }

  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)size << 30) | ((u32)op << 24) | ((u32)opc << 22) |
              ((u32)imm << 10) | ((u32)Rn.reg << 5) | Rt.reg;
  return assembled;
}

// WHAT THE FUCK v2
ArmInstruction assembleLdrStrReg(const Fields *instruction) {
  ArmInstruction assembled = 0;
  Register Rt, Rn, Rm;

  if (!parseRegister(instruction->fields[1].value, &Rt) ||
      !parseRegister(instruction->fields[3].value, &Rn) ||
      !parseRegister(instruction->fields[4].value, &Rm)) {
    errorFields("Arguments 1, 2, 3 must be registers", instruction);
  }

  if (Rt.reg == RSP || Rm.reg == RSP) {
    errorFields("Arguments 1 and 3 must be registers R0 - R29 or RZR/LR", instruction);
  }

  if (Rn.reg == RSP) {
    Rn.reg = REG_SP;
  }

  if (!Rn.extended) {
    errorFields("Argument 2 must have size of 64bits", instruction);
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
    errorFields(
        "ldrsw - accepts as Argument 1 only 64 bit register",
        instruction);
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
    errorFields(
        "Instruction accepts as argument 1 only 32 bit register",
        instruction);
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
      parseShift(instruction->fields[5].value, &sh);
      if (sh != SH_LSL) {
        errorFields("Only 'LSL' shift allowed", instruction);
      }
      if (size == 0) {
        if (!Rm.extended || instruction->fields[6].type != T_IMMEDIATE) {
          errorFields("Shift without parameter", instruction);
        }
      }
    } else if (arg == T_EXTEND) {
      parseExtend(instruction->fields[5].value, &ex);
      switch (ex) {
      case UXTW:
      case SXTW:
        if (Rm.extended) {
          errorFields(
              "If using UXTW or SXTW shiftS argument 4 must have size of 32bits",
              instruction);
        }
        option = 2;
        if (ex == SXTW) {
          option = BIT(2) | option;
        }
        break;
      case SXTX:
        if (!Rm.extended) {
          errorFields("If using SXTX shift argument 4 must have size of 64bits",
                      instruction);
        }
        option = 7;
        break;
      default:
        errorFields("Unknown shift", instruction);
      }
    }
    if (instruction->fields[6].type == T_IMMEDIATE) {
      if (!parseImmediateU8(instruction->fields[6].value, &imm)) {
        errorFields("Argument 5 must have type uint8", instruction);
      }
      if (imm != size && imm != 0) {
        errorFields(
            "Argument 5 must equals 0 or size (in bytes) specified by instruction. e.g ldrsb b - 1 byte",
            instruction);
      }
      if (imm == size) {
        S = BIT(2) | S;
      }
    }
  }

  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)size << 30) | ((u32)SOP_LDR_STR_REG << 24) |
              ((u32)opc << 21) | ((u32)Rm.reg << 16) | ((u32)option << 13) |
              ((u32)S << 10) | ((u32)Rn.reg << 5) | Rt.reg;
  return assembled;
}

// DONE
ArmInstruction assembleConditionalCompareImm(const Fields *instruction) {
  ArmInstruction assembled = 0;
  
  Register Rn;
  if (!parseRegister(instruction->fields[1].value, &Rn)) {
    errorFields("Argument 1 must be register R0 - R29 or RZR/LR", instruction);
  }

  if (Rn.reg == RSP) {
    errorFields("Argument 1 must be register R0 - R29 or RZR/LR", instruction);
  }

  bool sf = Rn.extended;

  u8 imm;
  if (!parseImmediateU8(instruction->fields[2].value, &imm)) {
    errorFields("Argument 2 must have type uint8", instruction);
  }

  if (imm > GENMASK(5)) {
    errorFields("Argument 2 must be in the range 0 to 31", instruction);
  }

  u8 imm_nzcv;
  if (!parseImmediateU8(instruction->fields[3].value, &imm_nzcv)) {
    errorFields("Argument 3 must have type uint8", instruction);
  }

  if (imm_nzcv > 15) {
    errorFields("Argument 3 must be in the range 0 to 15", instruction);
  }

  ConditionType cd;
  if (!parseCondition(instruction->fields[4].value, &cd)) {
    errorFields("Unknown condition", instruction);
  }

  u8 op = instructionIndex(CONDITIONAL_COMPARE_REG, instruction->fields->value);
  u8 S = 1;

  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)sf << 31) | ((u32)op << 30) | ((u32)S << 29) |
              ((u32)SOP_CONDITIONAL_COMPARE_IMM << 21) | ((u32)imm << 16) |
              ((u32)cd << 12) | ((u32)2 << 10) | ((u32)Rn.reg << 5) |
              imm_nzcv;
  return assembled;
}

// DONE
ArmInstruction assembleConditionalCompareReg(const Fields *instruction) {
  ArmInstruction assembled = 0;
  
  Register Rn, Rm;
  if (!parseRegister(instruction->fields[1].value, &Rn) ||
      !parseRegister(instruction->fields[2].value, &Rm)) {
    errorFields("Arguments 1 and 2 must be registers R0 - R29 or RZR/LR", instruction);
  }

  if (Rn.reg == RSP || Rm.reg == RSP) {
    errorFields("Arguments 1 and 2 must be registers R0 - R29 or RZR/LR", instruction);
  }

  bool sf;
  if (Rm.extended && Rn.extended) {
    sf = 1;
  } else if (!Rm.extended && !Rn.extended) {
    sf = 0;
  } else {
    errorFields("Registers must have same size", instruction);
  }

  u8 imm_nzcv;
  if (!parseImmediateU8(instruction->fields[3].value, &imm_nzcv)) {
    errorFields("Argument 3 must have type uint8", instruction);
  }

  if (imm_nzcv > 15) {
    errorFields("Argument 3 must be in the range 0 to 15", instruction);
  }

  ConditionType cd;
  if (!parseCondition(instruction->fields[4].value, &cd)) {
    errorFields("Unknown condition", instruction);
  }

  u8 op = instructionIndex(CONDITIONAL_COMPARE_REG, instruction->fields->value);
  u8 S = 1;

  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)sf << 31) | ((u32)op << 30) | ((u32)S << 29) | 
              ((u32)SOP_CONDITIONAL_COMPARE_REG << 21) | ((u32)Rm.reg << 16) |
              ((u32)cd << 12) | ((u32)Rn.reg << 5) | imm_nzcv;
  return assembled;
}

// DONE
ArmInstruction assembleLdrLiteral(const Fields *instruction) {
  ArmInstruction assembled = 0;
  Register Rt = {0};
  if (!parseRegister(instruction->fields[1].value, &Rt)) {
    errorFields("Argument 1 must be register R0 - R29 or RZR/LR", instruction);
  }

  if (Rt.reg == RSP) {
    errorFields("Argument 1 must be register R0 - R29 or RZR/LR", instruction);
  }

  u8 opc = instructionIndex(LDR_LITERAL, instruction->fields->value) + 1;
  if (!Rt.extended && opc == 2) {
    errorFields("Instruction ldrsw supports only 64 bits registers", instruction);
  }
  if (!Rt.extended && opc == 1) {
    opc = 0;
  }

  i32 offset = 0;
  i16 label_section = getLabelSection(instruction->fields[2].value);
  if (label_section == -1) {
    errorFields("Argument 2 - unknown label", instruction);
  }

  if (label_section == CONTEXT.cur_sndx) {
    i64 label_pc = getLabelPc(instruction->fields[2].value);
    const u8 offset_size = 19;
    offset = (label_pc - CONTEXT.pc) / ARM_INSTRUCTION_SIZE;

    offset = GENMASK(offset_size) & offset;
    if (offset < 0) {
      offset |= BIT(offset_size - 1);
    }
  } else {
    addRelocation(instruction->fields[2].value, R_AARCH64_LD_PREL_LO19);
  }

  if (ERRORS) {
    return 0;
  }
  assembled =
      ((u32)opc << 30) | ((u32)SOP_LDR_LITERAL << 24) | (offset << 5) | Rt.reg;
  return assembled;
}

// DONE
ArmInstruction assembleUnconditionalBranchReg(const Fields *instruction) {
  ArmInstruction assembled = 0;
  Register Rn = {true, RLR};

  u8 op =
      instructionIndex(UNCONDITIONAL_BRANCH_REG, instruction->fields[0].value);
  if (instruction->n_fields == 2 && op != 2) {
    errorFields("Not enough arguments", instruction);
  }

  if (instruction->n_fields != 2) {
    if (!parseRegister(instruction->fields[1].value, &Rn)) {
      errorFields("Argument 1 must be register R0 - R29 or RZR/LR",
                  instruction);
    }

    if (Rn.reg == RSP) {
      errorFields("Argument 1 must be register R0 - R29 or RZR/LR",
                  instruction);
    }

    if (!Rn.extended) {
      errorFields("Registers must have size of 64bits", instruction);
    }
  }

  const u8 Z = 0;
  const u16 op2 = 0xf80;

  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)SOP_UNCONDITIONAL_BRANCH_REG << 25) | ((u32)Z << 24) |
              ((u32)op << 21) | ((u32)op2 << 9) | ((u32)Rn.reg << 5) | 0;
  return assembled;
}

// DONE
ArmInstruction assembleUnconditionalBranchImm(const Fields *instruction) {
  ArmInstruction assembled = 0;

  u8 op =
      instructionIndex(UNCONDITIONAL_BRANCH_IMM, instruction->fields[0].value);

  i32 offset = 0;
  i16 label_section = getLabelSection(instruction->fields[1].value);
  if (label_section == -1) {
    errorFields("Argument 1 - unknown label", instruction);
  }

  if (label_section == CONTEXT.cur_sndx) {
    i64 label_pc = getLabelPc(instruction->fields[1].value);

    const u8 offset_size = 26;
    offset = (label_pc - CONTEXT.pc) / ARM_INSTRUCTION_SIZE;

    offset = GENMASK(offset_size) & offset;
    if (offset < 0) {
      offset |= BIT(offset_size - 1);
    }
  } else {
    addRelocation(instruction->fields[1].value,
                  op ? R_AARCH64_CALL26 : R_AARCH64_JUMP26);
  }

  assembled =
      ((u32)op << 31) | ((u32)SOP_UNCONDITIONAL_BRANCH_IMM << 26) | offset;
  return assembled;
}

// DONE
ArmInstruction assembleCompareBranch(const Fields *instruction) {
  ArmInstruction assembled = 0;
  Register Rt;

  if (!parseRegister(instruction->fields[1].value, &Rt)) {
    errorFields("Argument 1 must be register R0 - R29 or RZR/LR", instruction);
  }

  if (Rt.reg == RSP) {
    errorFields("Argument 1 must be register R0 - R29 or RZR/LR",
                instruction);
  }

  bool sf = Rt.extended;

  i32 offset = 0;
  i16 label_section = getLabelSection(instruction->fields[2].value);
  if (label_section == -1) {
    errorFields("Argument 2 - unknown label", instruction);
  }

  if (label_section == CONTEXT.cur_sndx) {
    i64 label_pc = getLabelPc(instruction->fields[2].value);

    const u8 offset_size = 19;
    offset = (label_pc - CONTEXT.pc) / ARM_INSTRUCTION_SIZE;

    offset = GENMASK(offset_size) & offset;
    if (offset < 0) {
      offset |= BIT(offset_size - 1);
    }
  } else {
    addRelocation(instruction->fields[2].value, R_AARCH64_CONDBR19);
  }

  u8 op = instructionIndex(COMPARE_BRANCH, instruction->fields[0].value);

  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)sf << 31) | ((u32)SOP_COMPARE_BRANCH << 25) |
              ((u32)op << 24) | ((u32)offset << 5) | Rt.reg;
  return assembled;
}

// DONE
ArmInstruction assembleTestBranch(const Fields *instruction) {
  ArmInstruction assembled = 0;
  Register Rt;

  if (!parseRegister(instruction->fields[1].value, &Rt)) {
    errorFields("Argument 1 must be register R0 - R29 or RZR/LR", instruction);
  }

  if (Rt.reg == RSP) {
    errorFields("Argument 1 must be register R0 - R29 or RZR/LR",
                instruction);
  }

  u8 imm;
  if (!parseImmediateU8(instruction->fields[2].value, &imm)) {
    errorFields("Argument 2 must be 0 to (register size - 1)", instruction);
  }

  bool sf = Rt.extended;
  bool imm_in_range = sf ? imm < 64 : imm < 32;
  if (!imm_in_range) {
    errorFields("Argument 2 must be 0 to (register size - 1)", instruction);
  }

  imm = imm & 0x1f;

  i32 offset = 0;
  i16 label_section = getLabelSection(instruction->fields[3].value);
  if (label_section == -1) {
    errorFields("Argument 3 - unknown label", instruction);
  }

  if (label_section == CONTEXT.cur_sndx) {
    i64 label_pc = getLabelPc(instruction->fields[3].value);

    const u8 offset_size = 14;
    offset = (label_pc - CONTEXT.pc) / ARM_INSTRUCTION_SIZE;

    offset = GENMASK(offset_size) & offset;
    if (offset < 0) {
      offset |= BIT(offset_size - 1);
    }
  } else {
    addRelocation(instruction->fields[3].value, R_AARCH64_TSTBR14);
  }

  u8 op = instructionIndex(TEST_BRANCH, instruction->fields[0].value);

  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)sf << 31) | ((u32)SOP_TEST_BRANCH << 25) | ((u32)op << 24) |
              ((u32)imm << 19) | ((u32)offset << 5) | Rt.reg;
  return assembled;
}

// DONE
ArmInstruction assembleConditionalBranchImm(const Fields *instruction) {
  ArmInstruction assembled = 0;
  ConditionType c = 0;
  if (!parseCondition(instruction->fields[1].value, &c)) {
    errorFields("Unknown condition", instruction);
  }

  i32 offset = 0;
  i16 label_section = getLabelSection(instruction->fields[2].value);
  if (label_section == -1) {
    errorFields("Argument 2 - unknown label", instruction);
  }

  if (label_section == CONTEXT.cur_sndx) {
    i64 label_pc = getLabelPc(instruction->fields[2].value);

    const u8 offset_size = 19;
    offset = (label_pc - CONTEXT.pc) / ARM_INSTRUCTION_SIZE;

    offset = GENMASK(offset_size) & offset;
    if (offset < 0) {
      offset |= BIT(offset_size - 1);
    }
  } else {
    addRelocation(instruction->fields[2].value, R_AARCH64_CONDBR19);
  }

  u8 o = 0;

  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)SOP_CONDITIONAL_BRANCH_IMM << 24) | ((u32)offset << 5) |
              (o << 4) | c;
  return assembled;
}

// DONE
ArmInstruction assemblePcRelAddressing(const Fields *instruction) {
  ArmInstruction assembled = 0;
  Register Rd;

if (!parseRegister(instruction->fields[1].value, &Rd)) {
    errorFields("Argument 1 must be register R0 - R29 or RZR/LR", instruction);
  }

  if (Rd.reg == RSP) {
    errorFields("Argument 1 must be register R0 - R29 or RZR/LR", instruction);
  }

  if (!Rd.extended) {
    errorFields("Registers must have size of 64bits", instruction);
  }

  i32 offset = 0;
  i16 label_section = getLabelSection(instruction->fields[2].value);
  if (label_section == -1) {
    errorFields("Argument 2 - unknown label", instruction);
  }

  if (label_section == CONTEXT.cur_sndx) {
    i64 label_pc = getLabelPc(instruction->fields[2].value);

    const u8 offset_size = 21;
    offset = label_pc - CONTEXT.pc;

    offset = GENMASK(offset_size) & offset;
    if (offset < 0) {
      offset |= BIT(offset_size - 1);
    }
  } else {
    addRelocation(instruction->fields[2].value, R_AARCH64_ADR_PREL_LO21);
  }

  u8 op = instructionIndex(PCRELADDRESSING, instruction->fields->value);
  u8 immlo = offset & 3; // low 2 bits
  u32 immhi = offset >> 2; // remove low 2 bits

  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)op << 31) | ((u32)immlo << 29) | ((u32)SOP_PC_REl << 24) |
              ((u32)immhi << 5) | Rd.reg;
  return assembled;
}

// DONE
ArmInstruction assembleException(const Fields *instruction) {
  ArmInstruction assembled = 0;
  const char *mnemonic = instruction->fields->value;

  // base parameters to encode SVC, HVC, SMC
  u8 LL = instructionIndex(EXCEPTION, mnemonic) + 1;
  u8 opc = 0; 

  u16 imm = 0;
  if (*instruction->fields->value == 'D') {
    opc = 5; // 0b101 - opc for DPCS
    LL = mnemonic[strlen(mnemonic) - 1] - '0';
  }

  if (*mnemonic == 'B') {
    opc = 1;
  }
  if (*mnemonic == 'H') {
    opc = 2;
  }
  if (*mnemonic == 'T') {
    opc = 3;
  }

  imm = 0;
  if (!parseImmediateU16(instruction->fields[1].value, &imm)) {
    errorFields("Argument 1 must have type uint16", instruction);
  }

  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)SOP_EXCEPTIONS << 24u) | ((u32)opc << 21u) |
              ((u32)imm << 5u) | LL;

  return assembled;
}

// DONE
ArmInstruction assembleLogicalShReg(const Fields *instruction) {
  ArmInstruction assembled = 0;

  Register Rd;
  Register Rn;
  Register Rm;

  if (!parseRegister(instruction->fields[1].value, &Rd) ||
      !parseRegister(instruction->fields[2].value, &Rn) ||
      !parseRegister(instruction->fields[3].value, &Rm)) {
    errorFields("Arguments 1, 2, 3 must be registers R0 - R29 or RZR/LR", instruction);
  }

  if (Rd.reg == RSP || Rn.reg == RSP || Rm.reg == RSP) {
    errorFields("Arguments 1, 2, 3 must be registers R0 - R29 or RZR/LR", instruction);
  }

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
  if (instruction->n_fields > 5 ) {
    if (!parseShift(instruction->fields[4].value, &t)) {
      errorFields("Unknown shift", instruction);
    }

    if (!parseImmediateU8(instruction->fields[5].value, &sh_imm)) {
      errorFields("Argument 5 must be x < register size && x >= 0", instruction);
    }
    if (sh_imm >= (sf ? 64 : 32)) {
      errorFields("Argument 5 must be 0 to (register size - 1)", instruction);
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

  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)sf << 31) | ((u32)opc << 29) |
              ((u32)SOP_LOGICAL_SH_REG << 24) | ((u32)t << 22) |
              ((u32)N << 21) | ((u32)Rm.reg << 16) | ((u32)sh_imm << 10) |
              ((u32)Rn.reg << 5) | Rd.reg;

  return assembled;
}

// DONE
ArmInstruction assembleMoveWide(const Fields *instruction) {
  ArmInstruction assembled = 0;
  Register Rd = {true, REG_SP};

  if (!parseRegister(instruction->fields[1].value, &Rd) || Rd.reg == REG_SP) {
    errorFields("Argument 1 must be register R0 - R29 or RZR/LR", instruction);
  }

  u16 imm = 0;
  if (!parseImmediateU16(instruction->fields[2].value, &imm)) {
    errorFields("Argument 2 must have type uint16", instruction);
  }

  bool sf = Rd.extended;

  ShiftType t = SH_LSL;
  u8 sh_imm = 0;
  if (instruction->n_fields > 4) {
    if (!parseShift(instruction->fields[3].value, &t)) {
      errorFields("Unknown shift", instruction);
    }

    if (t != SH_LSL) {
      errorFields("Only 'LSL' shift allowed", instruction);
    }

    if (!parseImmediateU8(instruction->fields[4].value, &sh_imm)) {
      errorFields("Argument 4 must have type uint8", instruction);
    }

    if (sf && (sh_imm > 48 || sh_imm % 16 != 0)) {
      errorFields("Argument 4 must be multiple of 16 and be less than 48", instruction);
    } else if (!sf && sh_imm != 0 && sh_imm != 16) {
      errorFields("Argument 4 must be 0 or 16", instruction);
    }
  }

  u8 opc = instructionIndex(MOVEWIDE, instruction->fields->value);
  if (opc != 0) { // because opc 0b01 not allocated
    opc += 1;
  }

  u8 hw = sh_imm / 16;

  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)sf << 31) | ((u32)opc << 29) | ((u32)SOP_MOVE_WIDE << 23) |
              ((u32)hw << 21) | ((u32)imm << 5) | Rd.reg;
  return assembled;
}

// DONE
ArmInstruction assembleAddSubImm(const Fields *instruction) {
  ArmInstruction assembled = 0;

  Register Rd;
  Register Rn;

  if (!parseRegister(instruction->fields[1].value, &Rd) ||
      !parseRegister(instruction->fields[2].value, &Rn)) {
    errorFields("Arguments 1 and 2 must be registers", instruction);
  }

  if (Rn.reg == RSP) {
    Rn.reg = REG_SP;
  }

  if (Rd.reg == RSP) {
    bool signed_ = instruction->fields->value[strlen(instruction->fields->value) - 1] == 's';
    if (!signed_) {
      Rd.reg = REG_SP;
    } else {
      errorFields("Argument 1 for instruction must be R0 - R29 or RZR/LR", instruction);
    }
  }

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
    errorFields("Argument 3 must be 0 to 4095", instruction);
  }

  if (imm > 4095) {
    errorFields("Argument 3 must be 0 to 4095", instruction);
  }

  ShiftType t = SH_LSL;
  u8 sh_imm = 0;
  if (instruction->n_fields > 5) {
    if (!parseShift(instruction->fields[4].value, &t)) {
      errorFields("Unknown shift", instruction);
    }

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

  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)sf << 31) | ((u32)op << 30) | ((u32)S << 29) |
              ((u32)SOP_ADD_SUB_IMM << 23) | ((u32)sh << 22) |
              ((u32)imm << 10) | ((u32)Rn.reg << 5) | Rd.reg;

  return assembled;
}

// DONE
ArmInstruction assembleLogicalImm(const Fields *instruction) {
  ArmInstruction assembled = 0;

  Register Rd;
  Register Rn;

  if (!parseRegister(instruction->fields[1].value, &Rd) ||
      !parseRegister(instruction->fields[2].value, &Rn)) {
    errorFields("Arguments 1 and 2 must be registers", instruction);
  }

  if (Rd.reg == RSP) {
    Rd.reg = REG_SP;
  }

  if (Rn.reg == RSP) {
    errorFields("Argument 2 must be register R0 - R29 or RZR/LR", instruction);
  }

  bool sf = 0;
  if (Rd.extended && Rn.extended) {
    sf = 1;
  } else if (!Rd.extended && !Rn.extended) {
    sf = 0;
  } else {
    errorFields("Registers must have same size", instruction);
  }

  u64 imm = 0;
  u64 encoding;
  if (!parseImmediateU64(instruction->fields[3].value, &imm)) {
    errorFields("Argument 3 must be immediate a 32-bit or 64-bit pattern of "
                "identical elements of size = 2, 4, 8, 16, 32, or 64 bits",
                instruction);
  } else {
    if (!encodeBitmaskImmediate(imm, &encoding, sf)) {
      errorFields("Argument 3 must be immediate a 32-bit or 64-bit pattern of "
                  "identical elements of size = 2, 4, 8, 16, 32, or 64 bits",
                  instruction);
    }
  }

  u8 opc = instructionIndex(LOGICAL_IMM, instruction->fields[0].value);

  if (ERRORS) {
    return 0;
  }
  assembled = ((u32)sf << 31) | ((u32)opc << 29) |
              ((u32)SOP_LOGICAL_IMM << 23) | (encoding << 10) |
              ((u32)Rn.reg << 5) | Rd.reg;
  return assembled;
}

ArmInstruction assemble(const Fields *instruction) {
  Signature s = getSignature(instruction);
  InstructionType it = getInstructionType(instruction->fields->value, &s);

  if (it == NONE) {
    i8 i = 0;
    if ((i = searchMnemonic(instruction->fields->value)) != -1) {
      it = INSTRUCTIONS[i].type;
      errorFields("Incorrect arguments for instruction", instruction);
    } else {
      errorFields("Unknown instruction", instruction);
    }
  }

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
  case LDR_STR_REG:
    i = assembleLdrStrReg(instruction);
    break;
  case CONDITIONAL_COMPARE_REG:
    i = assembleConditionalCompareReg(instruction);
    break;
  case CONDITIONAL_COMPARE_IMM:
    i = assembleConditionalCompareImm(instruction);
    break;
  case LDR_STR_IMM_POST_INDEX:
  case LDR_STR_IMM_PRE_INDEX:
  case LDR_STR_IMM_UNSIGNDE_OFFSET:
    i = assembleLdrStrImm(instruction, it);
    break;
  case NONE:
    return 0;
  }

  return i ? i : 0;
}
