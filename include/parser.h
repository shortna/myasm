#ifndef MYASM_PARSER
#define MYASM_PARSER

#include "myasm.h"
#define REGISTER_ZR_SP (31)

struct Token;

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

typedef enum {
  SH_LSL, 
  SH_LSR, 
  SH_ASR, 
  SH_ROR,
} ShiftType;

typedef enum {
  EQ,
  NE,
  CS,
  CC,
  MI,
  PL,
  VS,
  VC,
  HI,
  LS,
  GE,
  LT,
  GT,
  LE,
  AL,
  NV,
} ConditionType;

u8 parseLabelDeclaration(char *label);
u8 parseRegister(const char *reg, Register *r);
u8 parseImmediateU8(const char *imm, u8 *res);
u8 parseImmediateU16(const char *imm, u16 *res);
u8 parseImmediateU32(const char *imm, u32 *res);
u8 parseImmediateU64(const char *imm, u64 *res);
u8 parseExtend(const char *extend, ExtendType *ex);
u8 parseShift(const char *shift, ShiftType *sh);
u8 parseCondition(const char *condition, ConditionType *ct);

#endif
