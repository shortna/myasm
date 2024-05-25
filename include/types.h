#ifndef MYASM_TYPES
#define MYASM_TYPES

#include "myasm.h"

typedef u32 ArmInstruction;

#define BIT(p) (1ull << (p))

typedef enum {
  T_NONE              = BIT(0),
  T_EOL               = BIT(1),

  T_DIRECTIVE         = BIT(2),
  T_INSTRUCTION       = BIT(3),
  T_LABEL_DECLARATION = BIT(4),

  // args
  T_LABEL             = BIT(5),
  T_REGISTER          = BIT(6),
  T_IMMEDIATE         = BIT(7),
  T_SHIFT             = BIT(8),
  T_EXTEND            = BIT(9),
  T_CONDITION         = BIT(10),
  T_STRING            = BIT(11),

  // one symbol tokens
  T_RSBRACE           = BIT(12),
  T_LSBRACE           = BIT(13),
  T_BANG              = BIT(14),
  T_EQUAL             = BIT(15),
} TokenType;

typedef struct Token {
  TokenType type;
  u8 capacity;
  char *value;
} Token;

typedef struct Fields {
  Token fields[FIELDS_SIZE];
  u8 n_fields;
} Fields;

struct _IO_FILE;
typedef struct _IO_FILE FILE;

typedef struct Context {
  u64 pc;
  u64 size;
  u8 cur_sndx;
  FILE *cur_src;
  FILE *out;
} Context;
extern Context CONTEXT;

Fields initFields(size_t size_of_field);
void freeFields(Fields *fields);

Token initToken(size_t size);
void copyToken(Token *dst, const Token *src);
void resizeToken(Token *t);

#endif
