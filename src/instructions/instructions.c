#include "instructions.h"
#define NEED_REPLACE UINT8_MAX

static const Instruction INSTRUCTIONS[] = {
    {{"AND", "ORR", "EOR", "ANDS", 0},
     LOGICAL_IMM,
     .limm = (LogicalImm){0b1, NEED_REPLACE, SOP_LOGICAL_IMM, 0}},

    {{"AND", "BIC", "ORR", "ORN", "EOR", "EON", "ANDS", "BICS"},
     LOGICAL_SH_REG,
     .lshreg = (LogicalShReg){0b1, NEED_REPLACE, SOP_LOGICAL_SH_REG, 0}},

    {{"MOVN", "MOVZ", "MOVK"},
     MOVEWIDE,
     .movewide = (MoveWide){0b1, NEED_REPLACE, SOP_MOVE_WIDE, 0}},

    {{"ADD", "ADDS", "SUB", "SUBS"},
     ADDSUB_IMM,
     .addsub =
         (AddSubImm){0b1, NEED_REPLACE, NEED_REPLACE, SOP_ADD_SUB_IMM, 0}},

    {{"ADR", "ADRP"},
     PCRELADDRESSING,
     .pcreladdr = (PcRelAddressing){NEED_REPLACE, NEED_REPLACE, SOP_PC_REl, 0}},

    {{"SVC", "HVC", "SMC", "BRK", "HLT", "TCANCEL", "DCPS1", "DCPS2", "DCPS3"},
     EXCEPTION,
     .exception = (Exceptions){SOP_EXCEPTIONS, 0}},
};

const Instruction *searchInstruction(const char *mnemonic) {
  for (size_t i = 0; i < sizeof(INSTRUCTIONS) / sizeof(*INSTRUCTIONS); i++) {
    for (u8 j = 0; j < MNEMONICS_PER_GROUP; j++) {
      if (strcmp(INSTRUCTIONS[i].mnemonic[j], mnemonic) == 0) {
        return INSTRUCTIONS + i;
      }
    }
  }
  return NULL;
}
