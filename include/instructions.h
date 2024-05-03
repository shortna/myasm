#ifndef MYASM_ARM_INSTRUCTIONS
#define MYASM_ARM_INSTRUCTIONS

#define ARM_INSTRUCTION_SIZE (4)

#include "myasm.h"

struct Fields;
typedef struct Fields Fields;

u32 assemble(Fields *instruction);
u8 searchMnemonic(const char *mnemonic);

#ifdef __clang__
typedef enum : u8 {
#else
typedef enum {
#endif
  NO_ARG = 0,
  IMMEDIATE = 1,
  REGISTER = 2,
  SP = 4,
  LABEL = 8,
  SHIFT = 16,
  EXTEND = 32,
  BRACKETS = 64,
  OPTIONAL = 128,
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
} InstructionType;

typedef struct Instruction {
  char const *const mnemonic[10];
  const InstructionType type;
  const Signature s;
} Instruction;


#endif
