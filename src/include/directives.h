#ifndef MYASM_DIRECTIVES
#define MYASM_DIRECTIVES

#include "myasm.h"

struct Fields;
typedef struct Fields Fields;

u8 searchDirective(const char *name);
u8 execDirective(Fields *f);

#endif
