#ifndef MYASM_ARM_INSTRUCTIONS
#define MYASM_ARM_INSTRUCTIONS

#define ARM_INSTRUCTION_SIZE (4)

#include "myasm.h"

u32 assemble(const Fields *instruction);
u8 searchMnemonic(const char *mnemonic);

#endif
