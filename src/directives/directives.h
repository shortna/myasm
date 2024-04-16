#ifndef MYASM_DIRECTIVES
#define MYASM_DIRECTIVES

#include "myasm.h"

// forward declaration
struct Fields;
u8 searchDirective(const char *name);
u8 execDirective(struct Fields *f);

#endif
