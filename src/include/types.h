#ifndef MYASM_TYPES
#define MYASM_TYPES

#include "myasm.h"
#define STR_START_CAPACITY UINT8_MAX

typedef struct Str {
  char *s;
  size_t len;
  size_t capacity;
} Str;

Str initStr(void);
void resizeStr(Str *str);
void concatStr(Str *str1, const Str *str2);
void concatCStr(Str *str1, const char *str2);
void toUpper(Str *line);

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

Fields initFields(size_t size_of_field);
void freeFields(Fields *fields);

#endif
