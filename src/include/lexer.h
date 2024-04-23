#ifndef MYASM_LEXER
#define MYASM_LEXER

#include "myasm.h"

// forward declaration
struct _IO_FILE;
typedef struct _IO_FILE FILE;

struct Token;
typedef struct Token Token;

Token initToken(size_t size);
void copyToken(Token *dst, const Token *src);
u8 getToken(FILE *f, Token *t);
u8 writeToken(FILE *f, const Token *t);
u8 readToken(FILE *f, Token *t);

#endif
