#ifndef MYASM_TYPES
#define MYASM_TYPES

#include "myasm.h"
#define TOKEN_SIZE (40)

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

typedef struct Token {
  TokenType type;
  size_t capacity;
  char *value;
} Token;

#define FIELDS_MAX (10)
typedef struct Fields {
  Token fields[FIELDS_MAX];
  u8 n_fields;
} Fields;

struct _IO_FILE;
typedef struct _IO_FILE FILE;

typedef struct Context {
  u64 pc;
  FILE *cur_src;
} Context;

extern Context CONTEXT;

Fields initFields(size_t size_of_field);
void freeFields(Fields *fields);

Token initToken(size_t size);
void copyToken(Token *dst, const Token *src);

#endif
