#ifndef MYASM_ASSEMBLE
#define MYASM_ASSEMBLE

#include "myasm.h"

// forward declaration
struct _IO_FILE;
typedef struct _IO_FILE FILE;

u8 make(FILE *src, const char *out_name);

#endif
