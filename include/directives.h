#ifndef MYASM_DIRECTIVES
#define MYASM_DIRECTIVES

#include "myasm.h"

struct Fields;
typedef struct Fields Fields;

typedef enum {
  D_NONE,
  D_GLOBAL,
  D_SECTION,
  D_BYTE,
  D_INT,
  D_ASCII,
  D_ASCIIZ,
  D_ZERO,
} DirectiveType;

struct Directive {
  const char *name;
  DirectiveType t;
};

static const struct Directive DIRECTIVES[] = {
  {"global", D_GLOBAL},
  {"section", D_SECTION},
  {"byte", D_BYTE},
  {"int", D_INT},
  {"ascii", D_ASCII},
  {"asciiz", D_ASCIIZ},
  {"zero", D_ZERO},
};


DirectiveType searchDirective(const char *name);
u8 execDirective(Fields *f); 
u8 getDirectiveSize(const char *directive_arg);

#endif
