#ifndef MYASM_ARM_INSTRUCTIONS_API
#define MYASM_ARM_INSTRUCTIONS_API

#define ARM_INSTRUCTION_SIZE (4)

#include "myasm.h"

struct Fields;
typedef struct Fields Fields;

typedef enum InstructionType {
  NONE,
  LOGICAL_IMM,
  LOGICAL_SH_REG,
  MOVEWIDE,
  ADDSUB_IMM,
  PCRELADDRESSING,
  EXCEPTION,
} InstructionType;

u32 assemble(Fields *instruction);
InstructionType searchMnemonic(const char *mnemonic);

#endif
