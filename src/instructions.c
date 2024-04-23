#include "instructions_api.h"
#include "instructions.h"
#include "types.h"
#include "parser.h"
#include "tables.h"
#include <string.h>

// Static opcodes
#define SOP_LOGICAL_IMM (36)    // (0b00100100)
#define SOP_LOGICAL_SH_REG (10) // (0b00001010)
#define SOP_MOVE_WIDE (37)      // (0b00100101)
#define SOP_ADD_SUB_IMM (34)    // (0b00100010)
#define SOP_PC_REl (16)         // (0b00010000)
#define SOP_EXCEPTIONS (212)    // (0b11010100)

// IMPORTANT
// instruction mnemonic MUST be in order of incresing opc field
static const Instruction INSTRUCTIONS[] = {
    {{"ADR", "ADRP"}, PCRELADDRESSING, {2, REGISTER, LABEL}},
    {{"MOVN", "MOVZ", "MOVK"}, MOVEWIDE, {3, REGISTER, IMMEDIATE, SHIFT | OPTIONAL}},

    {{"ADD", "SUB"}, ADDSUB_IMM, {4, REGISTER | SP, REGISTER | SP, IMMEDIATE, SHIFT | OPTIONAL}},
    {{"ADDS", "SUBS"}, ADDSUB_IMM, {4, REGISTER, REGISTER | SP, IMMEDIATE, SHIFT | OPTIONAL}},

    {{"AND", "ORR", "EOR", "ANDS"}, LOGICAL_IMM, {3, REGISTER | SP, REGISTER, IMMEDIATE}},
    {{"AND", "BIC", "ORR", "ORN", "EOR", "EON", "ANDS", "BICS"}, LOGICAL_SH_REG, {4, REGISTER, REGISTER, REGISTER, SHIFT | OPTIONAL}},
    {{"SVC", "HVC", "SMC", "BRK", "HLT", "TCANCEL"}, EXCEPTION, {1, IMMEDIATE}},
    {{"DCPS1", "DCPS2", "DCPS3"}, EXCEPTION, {1, IMMEDIATE | OPTIONAL}},
};

u8 searchMnemonic(const char *mnemonic) {
  for (size_t i = 0; i < sizeof(INSTRUCTIONS) / sizeof(*INSTRUCTIONS); i++) {
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

u32 assemblePcRelAddressing(Fields *instruction) {
  (void) instruction;
  return 0; 
}


// SHAMELESSLY STOLEN FROM LLVM

// Is this number's binary representation all 1s?
u8 isMask(u64 imm) {
   return ((imm + 1) & imm) == 0;
}

// Is this number's binary representation one or more 1s followed by
// one or more 0s?
u8 isShiftedMask(u64 imm) {
   return isMask((imm - 1) | imm);
}

#define countTrailingZeros(x) __builtin_ctzll(x)
#define countTrailingOnes(x)  __builtin_ctzll (~x)
#define countLeadingOnes(x) __builtin_clzll (~x)

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
  assert(size > ctz && "I should be smaller than element size");
  u32 immr = (size - ctz) & (size - 1);

  // If size has a 1 in the n'th bit, create a value that has zeroes in
  // bits [0, n] and ones above that.
  u64 nimms = ~(size-1) << 1;

  // Or the CTO value into the low bits, which must be below the Nth bit
  // bit mentioned above.
  nimms |= (cto-1);

  // Extract the seventh bit and toggle it to create the N field.
  u8 N = ((nimms >> 6) & 1) ^ 1;

  *encoding = (N << 12) | (immr << 6) | (nimms & 0x3f);
  return true;
}

u32 assembleLogicalImm(Fields *instruction) {
  LogicalImm i = {0};
  u32 assembled_instruction = 0;

  Register Rd;
  Register Rn;

  if (!parseRegister(instruction->fields[1].value, &Rd) ||
      !parseRegister(instruction->fields[2].value, &Rn)) {
    // error here
    return 0;
  }


  if (Rd.extended && Rn.extended) {
    i.sf = 1;
  } else if (!Rd.extended && !Rn.extended) {
    i.sf = 0;
  } else {
    // error here
    return 0;
  }

  u64 imm = 0;
  if (!parseImmediateU64(instruction->fields[3].value, &imm)) {
    // error here
    return 0;
  }

  u64 encoding;
  if (!encodeBitmaskImmediate(imm, &encoding, i.sf)) {
    // error here
    return 0;
  }

  i.opc = instructionIndex(LOGICAL_IMM, instruction->fields[0].value);
  i.s_op = SOP_LOGICAL_IMM;

  i.Rn = Rn.n;
  i.Rd = Rd.n;

  assembled_instruction |= ((u32)i.sf << 31) | ((u32)i.opc << 29) |
                           ((u32)i.s_op << 23) | (encoding << 10) |
                           ((u32)i.Rn << 5) | i.Rd;
  return assembled_instruction;
}

u32 assembleLogicalShReg(Fields *instruction) {
  if (instruction->n_fields != 4 && instruction->n_fields != 6) {
    // error here
    return 0;
  }

  LogicalShReg i = {0};
  u32 assembled_instruction = 0;

  Register Rd;
  Register Rn;
  Register Rm;

  if (!parseRegister(instruction->fields[1].value, &Rd) ||
      !parseRegister(instruction->fields[2].value, &Rn) ||
      !parseRegister(instruction->fields[3].value, &Rm)) {
    // error here
    return 0;
  }

  if (Rd.extended && Rn.extended && Rm.extended) {
    i.sf = 1;
  } else if (!Rd.extended && !Rn.extended && !Rm.extended) {
    i.sf = 0;
  } else {
    // error here
    return 0;
  }

  ShiftType t = SH_LSL;
  u8 imm = 0;
  if (instruction->n_fields == 6) {
    if (!parseShift(instruction->fields[4].value, &t)) {
      // error here
      return 0;
    }
    if (!parseImmediateU8(instruction->fields[5].value, &imm)) {
      // error here
      return 0;
    }
    if (imm >= (i.sf ? 64 : 32)) {
      // error here
      return 0;
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

  i.opc = instructionIndex(LOGICAL_SH_REG, instruction->fields[0].value);
  i.N = i.opc & 1;
  i.opc = i.opc >> 1;

  i.s_op = SOP_LOGICAL_SH_REG;

  i.Rd = Rd.n;
  i.Rn = Rn.n;
  i.Rm = Rm.n;

  i.sh = t;
  i.imm6 = imm;

  assembled_instruction |= ((u32)i.sf << 31) | ((u32)i.opc << 29) |
                           ((u32)i.s_op << 24) | ((u32)i.sh << 22) |
                           ((u32)i.N << 21) | ((u32)i.Rm << 16) |
                           ((u32)i.imm6 << 10) | ((u32)i.Rn << 5) | i.Rd;

  return assembled_instruction;
}

u32 assembleMoveWide(Fields *instruction) {
  MoveWide i = {0};
  u32 assembled_instruction = 0;

  Register Rd;
  if (!parseRegister(instruction->fields[1].value, &Rd)) {
    // error here
    return 0;
  }

  u16 res = 0;
  if (!parseImmediateU16(instruction->fields[2].value, &res)) {
    // error here
    return 0;
  }

  i.sf = Rd.extended;

  ShiftType t = SH_LSL;
  u8 imm = 0;
  if (instruction->n_fields > 3) {
    if (!parseShift(instruction->fields[3].value, &t)) {
      // error here
      return 0;
    }

    if (t != SH_LSL) {
      // error here
      return 0;
    }

    if (!parseImmediateU8(instruction->fields[4].value, &imm)) {
      // error here
      return 0;
    }

    if (i.sf && (imm > 48 || imm % 16 != 0)) {
      // error here
      return 0;
    } else if (!i.sf && imm != 0 && imm != 16) {
      // error here
      return 0;
    }
  }

  i.opc = instructionIndex(MOVEWIDE, instruction->fields->value);
  if (i.opc != 0) { // because opc 0b01 not allocated
    i.opc += 1;
  }

  i.s_op = SOP_MOVE_WIDE;
  i.hw = imm / 16;
  i.Rd = Rd.n;
  i.imm16 = res;

  assembled_instruction |= ((u32)i.sf << 31u) | ((u32)i.opc << 29u) |
                           ((u32)i.s_op << 23u) | ((u32)i.hw << 21u) |
                           ((u32)i.imm16 << 5u) | i.Rd;

  return assembled_instruction;
}

u32 assembleAddSubImm(Fields *instruction) {
  if (instruction->n_fields != 4 && instruction->n_fields != 6) {
    // error here
    return 0;
  }

  AddSubImm i = {0};
  u32 assembled_instruction = 0;

  Register Rd;
  Register Rn;

  if (!parseRegister(instruction->fields[1].value, &Rd) ||
      !parseRegister(instruction->fields[2].value, &Rn)) {
    // error here
    return 0;
  }

  if (Rd.extended && Rn.extended) {
    i.sf = 1;
  } else if (!Rd.extended && !Rn.extended) {
    i.sf = 0;
  } else {
    // error here
    return 0;
  }

  u16 imm;
  if (!parseImmediateU16(instruction->fields[3].value, &imm)) {
    // error here
    return 0;
  }

  if (imm > 4095) {
    // error here
    return 0;
  }

  ShiftType t = SH_LSL;
  u8 sh_imm = 0;
  if (instruction->n_fields == 6) {
    if (!parseShift(instruction->fields[4].value, &t)) {
      // error here
      return 0;
    }

    if (!parseImmediateU8(instruction->fields[5].value, &sh_imm)) {
      // error here
      return 0;
    }

    if (t != SH_LSL || sh_imm != 12) {
      // error here
      return 0;
    }
  }

  i.op = instructionIndex(ADDSUB_IMM, instruction->fields[0].value);
  i.S = *(instruction->fields->value + 3) != '\0';

  i.s_op = SOP_ADD_SUB_IMM;

  i.Rd = Rd.n;
  i.Rn = Rn.n;

  i.sh = sh_imm == 12;
  i.imm12 = imm;

  assembled_instruction |= ((u32)i.sf << 31u) | ((u32)i.op << 30u) |
                           ((u32)i.S << 29u) | ((u32)i.s_op << 23u) |
                           ((u32)i.sh << 22u) | ((u32)i.imm12 << 10u) |
                           ((u32)i.Rn << 5u) | i.Rd;

  return assembled_instruction;
}

u32 assembleException(Fields *instruction) {
  Exceptions i = {0};
  u32 assembled_instruction = 0;

  i.LL = instructionIndex(EXCEPTION, instruction->fields[0].value) + 1;
  i.opc = 5; // 0b101

  u16 imm = 0;
  if (instruction->n_fields > 1) {
    if (!parseImmediateU16(instruction->fields[1].value, &imm)) {
      // error here
      return 0;
    }

    // opc  LL  mnemonic
    // 101  01  DCPS1
    // 101  10  DCPS2
    // 101  11  DCPS3
    // do not process DCPS* instructions since the already set
    if (*instruction->fields[0].value != 'D') {
      // opc  LL  mnemonic
      // 000  01  SVC
      // 000  10  HVC
      // 000  11  SMC
      // LL already set
      i.opc = 0;
      if (i.LL - 1 > 2) {
        // opc  LL  0bLL  0bLL - 1  (0bLL - 1) >> 1  mnemonic
        // 001  4   0100  0011      0001             BRK
        // 010  5   0101  0100      0010             HLT
        // 010  6   0110  0101      0010             TCANCEL
        i.opc = (i.LL - 1) >> 1;
        i.LL = 0;
      }
    }
  }

  i.sop = SOP_EXCEPTIONS;
  i.imm16 = imm;
  i.op2 = 0;

  assembled_instruction |= ((u32)i.sop << 24u) | ((u32)i.opc << 21u) |
                           ((u32)i.imm16 << 5u) | (i.op2 << 2u) | i.LL;

  return assembled_instruction;
}

u8 compareSignatures(const Signature *s1, const Signature *s2) {
  u8 i = 0;
  int *s1_arr = ((int *)s1) + 1;
  int *s2_arr = ((int *)s2) + 1;
  while (i < s1->n_args) {
    // if args differs from what expected and optional not set
    if ((s1_arr[i] & s2_arr[i]) == 0 && !(s1_arr[i] & OPTIONAL)) {
      return 0;
    }
    i++;
  }

  return 1;
}

InstructionType getInstructionType(const char *mnemonic, Signature *s) {
  for (size_t i = 0; i < sizeof(INSTRUCTIONS) / sizeof(*INSTRUCTIONS); i++) {
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
  int *s_arr = ((int *)&s);

  for (u8 i = 1; i < instruction->n_fields; i++) {
    switch (instruction->fields[i].type) {
    case T_REGISTER: {
      Register r;
      parseRegister(instruction->fields[i].value, &r);
      if (r.n == REGISTER_ZR_SP) {
        if (strcmp(instruction->fields[i].value + 1, "ZR") != 0) {
          s_arr[i] = SP;
          break;
        }
      }
      s_arr[i] = REGISTER;
      break;
    }
    case T_LABEL:
      if (searchInSym(instruction->fields[i].value) == -1) {
        // error here
        break;
      }
      s_arr[i] = LABEL;
      break;
    case T_IMMEDIATE:
      s_arr[i] = IMMEDIATE;
      break;
    case T_SHIFT:
      if (i + 1 < instruction->n_fields &&
          instruction->fields[i + 1].type == T_IMMEDIATE) {
        s_arr[i] = SHIFT;
        break;
      }
      // error here (shift without parameter)
      break;
    case T_EXTEND:
      s_arr[i] = EXTEND;
      break;

      /*
      #warning "replace strcat in future or check length before concatinating"
          case T_RSBRACE:
          case T_LSBRACE:
            s_arr[i] = BRACKETS;
            u8 j = i;
            do {
              j++;
              strcat(instruction->fields[i].value,
      instruction->fields[j].value); } while (j < instruction->n_fields &&
                     instruction->fields[j].type != T_LSBRACE);

            if (instruction->fields[j].type != T_LSBRACE) {
              s_arr[i] = NONE;
              // error here
              break;
            }

            if (j + 1 < instruction->n_fields &&
                instruction->fields[j + 1].type == T_BANG) {
              strcat(instruction->fields[i].value, instruction->fields[j +
      1].value);
            }
            break;

          case T_PLUS:
          case T_MINUS:
            if (i + 1 < instruction->n_fields &&
                instruction->fields[i + 1].type == T_IMMEDIATE) {
              s_arr[i] = IMMEDIATE;
              strcat(instruction->fields[i].value, instruction->fields[i +
      1].value); break;
            }
            // error here (sign without parameter)
            break;

          case T_DOLLAR:
      #warning "figure out dollar"
            break;
          */
    }
  }

  s.n_args = instruction->n_fields - 1;
  return s;
}

u32 assemble(Fields *instruction) {
  if (!instruction || instruction->n_fields == 0) { // sanity check
    return 0;
  }

  Signature s = decodeTokens(instruction);
  InstructionType it = getInstructionType(instruction->fields->value, &s);

  u32 i = 0;
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
  case PCRELADDRESSING:
    i = assemblePcRelAddressing(instruction);
    break;
  case EXCEPTION:
    i = assembleException(instruction);
    break;
  case NONE:
    // error here
    return 0;
  }

  return i ? i : 0;
}
