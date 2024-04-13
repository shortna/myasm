#include "instructions.h"
#include "parse.h"
#include "../assemble.h"
#include <string.h>

// Static opcodes
#define SOP_LOGICAL_IMM (36)    // (0b00100100)
#define SOP_LOGICAL_SH_REG (10) // (0b00001010)
#define SOP_MOVE_WIDE (37)      // (0b00100101)
#define SOP_ADD_SUB_IMM (34)    // (0b00100010)
#define SOP_PC_REl (16)         // (0b00010000)
#define SOP_EXCEPTIONS (212)    // (0b11010100)

u8 INSTRUCTION_SIZE = ARM_INSTRUCTION_SIZE;

// IMPORTANT
// instruction mnemonic MUST be in order of incresing opc field
static const Instruction INSTRUCTIONS[] = {
    {{"ADR", "ADRP"}, PCRELADDRESSING, {2, REGISTER, LABEL}},
    {{"MOVN", "MOVZ", "MOVK"}, MOVEWIDE, {3, REGISTER, IMMEDIATE, SHIFT | OPTIONAL}},
    {{"ADD", "ADDS", "SUB", "SUBS"}, ADDSUB_IMM, {4, REGISTER | SP, REGISTER | SP, IMMEDIATE, SHIFT | OPTIONAL}},
    {{"AND", "ORR", "EOR", "ANDS"}, LOGICAL_IMM, {3, REGISTER | SP, REGISTER, IMMEDIATE}},
    {{"AND", "BIC", "ORR", "ORN", "EOR", "EON", "ANDS", "BICS"}, LOGICAL_SH_REG, {4, REGISTER, REGISTER, REGISTER, SHIFT | OPTIONAL}},
    {{"SVC", "HVC", "SMC", "BRK", "HLT", "TCANCEL"}, EXCEPTION, {1, IMMEDIATE}},
    {{"DCPS1", "DCPS2", "DCPS3"}, EXCEPTION, {1, IMMEDIATE | OPTIONAL}},
};

// s1 must be from INSTRUCTIONS
// second signature compares against first, order matters
bool compareSigantures(Signature s1, Signature s2) {
  u8 i = 0;
  u8 *s1_arr = ((u8 *)&s1) + 1;
  u8 *s2_arr = ((u8 *)&s2) + 1;
  while (i < s1.n_args) {
    // if args differs from what expected and optional not set
    if ((s1_arr[i] & s2_arr[i]) == 0 && !(s1_arr[i] & OPTIONAL)) {
      return false;
    }
    i++;
  }

  return true;
}

Signature getSignature(const fields_t *instruction) {
  Signature s = {0};

  if (instruction->n_fields == 1) {
    return s;
  }

  uint8_t *s_arr = ((uint8_t *)&s) + 1;
  const Str *f = instruction->fields + 1;

  for (u8 i = 0; i < instruction->n_fields - 1; i++) {
    s_arr[i] = getArgumentType(f + i);
  }
  s.n_args = instruction->n_fields - 1;

  return s;
}

InstructionType searchInstruction(const fields_t *fields) {
  Signature s = getSignature(fields);
  const char *mnemonic = fields->fields->s;

  for (size_t i = 0; i < sizeof(INSTRUCTIONS) / sizeof(*INSTRUCTIONS); i++) {
    if (compareSigantures(INSTRUCTIONS[i].s, s)) {
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

u8 instructionIndex(InstructionType type, const Str *mnemonic) {
  for (u8 i = 0; i < sizeof(INSTRUCTIONS) / sizeof(*INSTRUCTIONS); i++) {
    if (INSTRUCTIONS[i].type == type) {
      u8 j = 0;
      while (INSTRUCTIONS[i].mnemonic[j]) {
        if (strcmp(INSTRUCTIONS[i].mnemonic[j], mnemonic->s)) {
          return j;
        }
        j++;
      }
    }
  }

  return 0;
}

u8 encodeLogicalImmediate(u64 imm, u8 *N, u8 *immr, u8 *imms) {
  assert(0 && "encodeLogicalImmediate Not Implemented");
}

u32 assembleLogicalImm(fields_t *instruction) {
  LogicalImm i = {0};
  u32 assembled_instruction = 0;

  Register Rd = parseRegister(instruction->fields + 1);
  Register Rn = parseRegister(instruction->fields + 2);

  if (Rd.n == -1 || Rn.n == -1) {
    // error here
    return 0;
  }

  u64 imm = 0;
  if (!parseImmediateU64(instruction->fields + 3, &imm)) {
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

  if (!encodeLogicalImmediate(imm, &i.N, &i.immr, &i.imms)) {
    // error here
    return 0;
  }

  i.opc = instructionIndex(LOGICAL_IMM, instruction->fields);
  i.s_op = SOP_LOGICAL_IMM;

  i.Rn = Rn.n;
  i.Rd = Rd.n;

  assembled_instruction |= (i.sf << 31) | (i.opc << 29) | (i.s_op << 23) |
                           (i.N << 22) | (i.immr << 16) | (i.imms << 10) |
                           (i.Rn << 5) | i.Rd;
  return assembled_instruction;
}

u32 assembleLogicalShReg(fields_t *instruction) {
  LogicalShReg i = {0};
  u32 assembled_instruction = 0;

  Register Rd = parseRegister(instruction->fields + 1);
  Register Rn = parseRegister(instruction->fields + 2);
  Register Rm = parseRegister(instruction->fields + 3);

  if (Rd.n == -1 || Rn.n == -1 || Rm.n == -1) {
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

  Shift sh = {SH_LSL, 0};
  if (instruction->n_fields > 4) {
    if (!parseShift(instruction->fields + 4, instruction->fields + 5, &sh)) {
      // error here
      return 0;
    }
    if (i.sf && sh.imm > 63) {
      // error here
      return 0;
    } else if (!i.sf && sh.imm > 31) {
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

  i.opc = instructionIndex(LOGICAL_SH_REG, instruction->fields);
  i.N = i.opc & 1;
  i.opc = i.opc >> 1;

  i.s_op = SOP_LOGICAL_SH_REG;

  i.Rd = Rd.n;
  i.Rn = Rn.n;
  i.Rm = Rm.n;

  i.sh = sh.t;
  i.imm6 = sh.imm;

  assembled_instruction |= (i.sf << 31) | (i.opc << 29) | (i.s_op << 24) |
                           (i.sh << 22) | (i.N << 21) | (i.Rm << 16) |
                           (i.imm6 << 10) | (i.Rn << 5) | i.Rd;

  return assembled_instruction;
}

u32 assembleMoveWide(fields_t *instruction) {
  MoveWide i = {0};
  u32 assembled_instruction = 0;

  Register Rd = parseRegister(instruction->fields + 1);

  u16 res = 0;
  if (!parseImmediateU16(instruction->fields + 2, &res)) {
    // error here
    return 0;
  }

  i.sf = Rd.extended;

  Shift sh = {SH_LSL, 0};
  if (instruction->n_fields > 3) {
    if (!parseShift(instruction->fields + 3, instruction->fields + 4, &sh)) {
      // error here
      return 0;
    }

    if (sh.t != SH_LSL) {
      // error here
      return 0;
    }
    if (i.sf && (sh.imm > 48 || sh.imm % 16 != 0)) {
      // error here
      return 0;
    } else if (sh.imm != 0 && sh.imm != 16) {
      // error here
      return 0;
    }
  }

  i.opc = instructionIndex(MOVEWIDE, instruction->fields);
  if (i.opc != 0) { // because opc 0b01 not allocated
    i.opc += 1;
  }

  i.s_op = SOP_MOVE_WIDE;
  i.hw = sh.imm / 16;
  i.Rd = Rd.n;
  i.imm16 = res;

  assembled_instruction |= (i.sf << 31) | (i.opc << 29) | (i.s_op << 23) |
                           (i.hw << 21) | (i.imm16 << 5) | i.Rd;

  return assembled_instruction;
}

u32 assembleAddSubImm(fields_t *instruction) {
  AddSubImm i = {0};
  u32 assembled_instruction = 0;

  Register Rd = parseRegister(instruction->fields + 1);
  Register Rn = parseRegister(instruction->fields + 2);

  if (Rd.n == -1 || Rn.n == -1) {
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
  if (!parseImmediateU16(instruction->fields + 3, &imm)) {
    // error here
    return 0;
  }

  if (imm > 4095) {
    // error here
    return 0;
  }

  Shift sh = {SH_LSL, 0};
  if (instruction->n_fields > 4) {
    if (!parseShift(instruction->fields + 3, instruction->fields + 4, &sh)) {
      // error here
      return 0;
    }

    if (sh.t != SH_LSL || sh.imm != 12) {
      // error here
      return 0;
    }
  }

  // mnemonic   op S  j  jb
  // ADD        0  0  0  00
  // ADDS       0  1  1  01
  // SUB        1  0  2  10
  // SUBS       1  1  3  11

  i.op = instructionIndex(ADDSUB_IMM, instruction->fields);
  i.S = i.op & 1;
  i.op = i.op >> 1;

  i.s_op = SOP_ADD_SUB_IMM;

  i.Rd = Rd.n;
  i.Rn = Rn.n;

  i.sh = sh.imm == 12;
  i.imm12 = imm;

  assembled_instruction |= (i.sf << 31) | (i.op << 30) | (i.S << 29) |
                            (i.s_op << 23) | (i.sh << 22) | (i.imm12 << 10) |
                            (i.Rn << 5) | i.Rd;

  return assembled_instruction;
}

u32 assembleException(fields_t *instruction) {
  Exceptions i = {0};
  u32 assembled_instruction = 0;

  i.LL = instructionIndex(EXCEPTION, instruction->fields) + 1;
  i.opc = 5; // 0b101

  u16 imm = 0;
  if (instruction->n_fields > 1) {
    if (!parseImmediateU16(instruction->fields + 1, &imm)) {
      // error here
      return 0;
    }

    // opc  LL  mnemonic
    // 101  01  DCPS1
    // 101  10  DCPS2
    // 101  11  DCPS3
    // do not process DCPS* instructions since the already set
    if (*instruction->fields->s != 'D') {
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

  assembled_instruction |=
      (i.sop << 24) | (i.opc << 21) | (i.imm16 << 5) | (i.op2 << 2) | i.LL;

  return assembled_instruction;
}

u32 assemblePcRelAddressing(fields_t *instruction) {
  return 0;
}

u32 assemble(fields_t *instruction) {
  if (!instruction) { // sanity check
    return 0;
  }
  InstructionType type = searchInstruction(instruction);

  u32 i = 0;
  switch (type) {
  case NONE:
    // warning unknown instruction
    return 0;
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
  }

  return i;
}
