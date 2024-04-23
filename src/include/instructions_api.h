#ifndef MYASM_ARM_INSTRUCTIONS_API
#define MYASM_ARM_INSTRUCTIONS_API

#define ARM_INSTRUCTION_SIZE (4)

#include "myasm.h"

struct Fields;
typedef struct Fields Fields;

u32 assemble(Fields *instruction);
u8 searchMnemonic(const char *mnemonic);

#endif
