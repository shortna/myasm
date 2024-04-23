#ifndef MYASM_ASSEMBLE
#define MYASM_ASSEMBLE

#include "myasm.h"

// forward declaration
struct _IO_FILE;
typedef struct _IO_FILE FILE;

u8 firstPass(FILE *src);
u8 secondPass(void);

#endif
