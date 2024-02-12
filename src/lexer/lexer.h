#ifndef MYASM_LEXER
#define MYASM_LEXER

#include <limits.h>
#include "../myasm.h"
#define MAX_TOKEN_SIZE UINT8_MAX

typedef enum TokenType {
  NONE = 0,

  DIRECTIVE,
  INSTRUCTION_OR_LABEL,
  LABEL_DECLARATION,

  NUMBER,
  REGISTER,
  STRING,

  // literals
  DOT,
  R_SBRACE,
  L_SBRACE,
  BANG,

  STAR,
  PLUS,
  MINUS,
  EQUAL,
  R_SLASH,
} TokenType;

typedef struct Token {
  TokenType type;
  char value[MAX_TOKEN_SIZE];
  u8 len;
} Token;

i8 getToken(FILE *src, Token *token);

#endif

