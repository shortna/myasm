#ifndef MYASM_LEXER
#define MYASM_LEXER

#include "myasm.h"

// forward declarations
struct _IO_FILE;
typedef struct _IO_FILE FILE;

u8 getToken(FILE *f, Token *t);

#endif
