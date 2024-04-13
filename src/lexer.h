#ifndef MYASM_LEXER
#define MYASM_LEXER

#include <stdio.h>
#include "myasm.h"

typedef enum {
  T_NONE,

  T_DIRECTIVE,
  T_INSTRUCTION,
  T_LABEL_DECLARATION,

// args
  T_LABEL,
  T_REGISTER,
  T_IMMEDIATE,
  T_SHIFT,
  T_EXTEND,

// one symbol tokens
  T_PLUS,
  T_MINUS,
  T_RSBRACE,
  T_LSBRACE,
  T_BANG,
  T_DOLLAR,
} TokenType;

typedef struct {
  TokenType type;
  char *value;
} Token;

u8 getToken(FILE *f, Token *t);

#endif
