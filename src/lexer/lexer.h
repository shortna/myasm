#ifndef MYASM_LEXER
#define MYASM_LEXER

#include <limits.h>
#include "../myasm.h"
#define MAX_TOKEN_SIZE UINT8_MAX

typedef enum TokenType {
  NONE = 0,
  INSTRUCTION = 1,
  REGISTER = 2,
  ARGUMENT = 3,
  NUMBER = 4,
  LABEL_DECLARATION = 5,
  LABEL = 6,
  DIRECTIVE = 7,
} TokenType;

typedef struct Token {
  u8 len;
  TokenType type;
  char value[MAX_TOKEN_SIZE];
} Token;

i8 getToken(FILE *src, Token *token);

#endif

