#include "instructions.h"
#include "../parse.h"

// Static opcodes
#define SOP_LOGICAL_IMM (0b00100100)
#define SOP_LOGICAL_SH_REG (0b00001010)
#define SOP_MOVE_WIDE (0b00100101)
#define SOP_ADD_SUB_IMM (0b00100010)
#define SOP_PC_REl (0b00010000)
#define SOP_EXCEPTIONS (0b11010100)

static const Instruction INSTRUCTIONS[] = {
    {{"ADR", "ADRP"}, PCRELADDRESSING, {2, REGISTER, LABEL}},
    {{"MOVN", "MOVZ", "MOVK"}, MOVEWIDE, {3, REGISTER, IMMEDIATE, SHIFT | OPTIONAL}},
    {{"ADD", "ADDS", "SUB", "SUBS"}, ADDSUB_IMM, {4, REGISTER | SP, REGISTER | SP, IMMEDIATE, SHIFT | OPTIONAL}},
    {{"AND", "ORR", "EOR", "ANDS"}, LOGICAL_IMM, {3, REGISTER | SP, REGISTER, IMMEDIATE}},
    {{"AND", "BIC", "ORR", "ORN", "EOR", "EON", "ANDS", "BICS"}, LOGICAL_SH_REG, {4, REGISTER, REGISTER, REGISTER, SHIFT | OPTIONAL}},
    {{"SVC", "HVC", "SMC", "BRK", "HLT", "TCANCEL", "DCPS1", "DCPS2", "DCPS3"}, EXCEPTION, {1, IMMEDIATE}},
};

// s1 must be from INSTRUCTIONS variable
// second signature compares against first, order matters
bool compareSigantures(Signature s1, Signature s2) {
  u8 i = 0;
  u8 *s1_arr = ((u8 *)&s1) + 1;
  u8 *s2_arr = ((u8 *)&s2) + 1;
  while (i < s1.n_args) {
    if ((s1_arr[i] & s2_arr[i]) == 0 && (s1_arr[i] & OPTIONAL) == 0) {
      return false;
    }
    i++;
  }

  return true;
}

Signature getSignature(const fields_t *instruction) { 
  Signature s = {0};
  // case for NOP instruction
  if (instruction->n_fields == 1) {
    return s;
  }

  uint8_t *s_arr = (uint8_t*) &s;
  const Str *f = instruction->fields + 1;

  //  for (size_t i = 0; i < instruction->n_fields - 1; i++) {
  //    s_array[i] = ;
  //  }

  return s;
}

InstructionType searchInstruction(const fields_t *fields) {
  Signature s = getSignature(fields);
  const char *mnemonic = fields->fields[0].s;

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


/* 
u8 encodeLogicalImmediate(u64 imm, u8 *N, u8 *immr, u8 *imms) {
  assert(0 && "encodeLogicalImmediate Not Implemented");
}

u8 assembleLogicalImm(fields_t *instruction, u32 *assembled_instruction) {
  LogicalImm i = {0};
  *assembled_instruction = 0;

  Register Rd = parseRegister(instruction->fields + 1);
  Register Rn = parseRegister(instruction->fields + 2);

  if (Rd.n == -1 || Rn.n == -1) {
    // error here
    return 0;
  }

  i64 res = 0;
  if (!parseImmediate(instruction->fields + 3, &res)) {
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

  if (!encodeLogicalImmediate(res, &i.N, &i.immr, &i.imms)) {
    // error here
    return 0;
  }

  if (strcmp(instruction->fields->s, "AND") == 0) {
    i.opc = 0b00;
  } else if (strcmp(instruction->fields->s, "ORR") == 0) {
    i.opc = 0b01;
  } else if (strcmp(instruction->fields->s, "EOR") == 0) {
    i.opc = 0b10;
  } else {
    i.opc = 0b11;
  }
  i.s_op = SOP_LOGICAL_IMM;

  i.Rn = Rn.n;
  i.Rd = Rd.n;

  *assembled_instruction = i.sf << 31 | i.opc << 29 | i.s_op << 23 | i.N << 22 |
                           i.immr << 16 | i.imms << 10 | i.Rn << 5 | i.Rd;
  return 1;
}

u8 assembleLogicalShReg(fields_t *instruction, u32 *assembled_instruction) {}
u8 assembleMoveWide(fields_t *instruction, u32 *assembled_instruction) {}
u8 assembleAddSubImm(fields_t *instruction, u32 *assembled_instruction) {}
u8 assemblePcRelAddressing(fields_t *instruction, u32 *assembled_instruction) {}
u8 assembleException(fields_t *instruction, u32 *assembled_instruction) {}

u32 assemble(fields_t *instruction, InstructionTag type) {
  if (!instruction) { // sanity check
    return 0;
  }

  u32 i = 0;
  switch (type) {
  case NONE:
    // warning illegal instruction here
  case LOGICAL_IMM:
    assembleLogicalImm(instruction, &i);
    break;
  case LOGICAL_SH_REG:
    assembleLogicalShReg(instruction, &i);
    break;
  case MOVEWIDE:
    assembleMoveWide(instruction, &i);
    break;
  case ADDSUB_IMM:
    assembleAddSubImm(instruction, &i);
    break;
  case PCRELADDRESSING:
    assemblePcRelAddressing(instruction, &i);
    break;
  case EXCEPTION:
    assembleException(instruction, &i);
    break;
  }

  return i;
}*/
