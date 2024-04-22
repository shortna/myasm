#ifndef MYASM_LEXER
#define MYASM_LEXER

#include <stdio.h>
#include "myasm.h"
#define REGISTER_ZR_SP (31)

typedef enum {
  UXTB,
  UXTH,
  UXTW,
  UXTX,
  SXTB,
  SXTH,
  SXTW,
  SXTX,
  LSL,
} ExtendType;

typedef struct {
  bool extended;
  i8 n;
} Register;

typedef enum {SH_LSL = 0, SH_LSR = 1, SH_ASR = 2, SH_ROR = 3} ShiftType;

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

Token initToken(size_t size);
void copyToken(Token *dst, Token *src);
u8 getToken(FILE *f, Token *t);
u8 writeToken(FILE *f, Token *t);
u8 readToken(FILE *f, Token *t);

u8 parseRegister(const char *reg, Register *r);
u8 parseImmediateU8(const char *imm, u8 *res);
u8 parseImmediateU16(const char *imm, u16 *res);
u8 parseImmediateU32(const char *imm, u32 *res);
u8 parseImmediateU64(const char *imm, u64 *res);
u8 parseExtend(const char *extend, ExtendType *ex);
u8 parseShift(const char *shift, ShiftType *sh);

#endif
