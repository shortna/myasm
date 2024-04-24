#ifndef MYASM_DIRECTIVES
#define MYASM_DIRECTIVES

#include "myasm.h"

struct Fields;
typedef struct Fields Fields;

i8 searchDirective(const char *name);
u8 execDirective(Fields *f, size_t pc); 

#endif
