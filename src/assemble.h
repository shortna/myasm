#ifndef MYASM_ASSEMBLE
#define MYASM_ASSEMBLE

#include "types.h"
extern u8 INSTRUCTION_SIZE;

InstructionType searchInstruction(const fields_t *fields);
u32 assemble(fields_t *instruction);

#endif
