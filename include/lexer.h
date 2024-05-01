#ifndef MYASM_LEXER
#define MYASM_LEXER

#include "myasm.h"

// forward declaration
struct _IO_FILE;
typedef struct _IO_FILE FILE;

struct Token;
typedef struct Token Token;

extern size_t LINE;
u8 getToken(FILE *f, Token *t);

#endif
