#ifndef MYASM_ASSEMBLE
#define MYASM_ASSEMBLE

#include "myasm.h"

// forward declaration
struct Fields;
u8 searchMnemonic(const char *mnemonic);
u32 assemble(struct Fields *instruction);

#endif
