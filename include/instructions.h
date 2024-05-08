#ifndef MYASM_ARM_INSTRUCTIONS
#define MYASM_ARM_INSTRUCTIONS

#define ARM_INSTRUCTION_SIZE (4)

#include "myasm.h"

struct Fields;
typedef struct Fields Fields;

u32 assemble(const Fields *instruction);
u8 searchMnemonic(const char *mnemonic);

typedef enum {
  NO_ARG = 0,
  IMMEDIATE = 1,
  REGISTER = 2,
  SP = 4,
  LABEL = 8,
  SHIFT = 16,
  EXTEND = 32,
  BRACKETS = 64,
  CONDITION = 128,
  OPTIONAL = 1024,
} Argument;

typedef struct Signature {
  u8 n_args;
  Argument a1;
  Argument a2;
  Argument a3;
  Argument a4;
  Argument a5;
  Argument a6;
  Argument a7;
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
  LDR_STR_REG_SHIFT,
  LDR_STR_REG_EXTEND,
  LDR_STR_IMM,
} InstructionType;

typedef struct Instruction {
  char const *const mnemonic[10];
  const InstructionType type;
  const Signature s;
} Instruction;


#endif
