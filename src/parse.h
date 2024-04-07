#ifndef MYASM_PARSER
#define MYASM_PARSER

#include "types.h"

typedef struct {
  bool extended;
  i8 n;
} Register;

Register parseRegister(const Str *reg);

typedef enum { SH_LSL = 0, SH_LSR = 1, SH_ASR = 2, SH_ROR = 3 } ShiftType;

typedef struct {
  u8 imm;
  ShiftType t;
} Shift;

u8 parseShift(const Str *shift_type, const Str *amount, Shift *sh);

u8 parseImmediate(const Str *imm, i64 *res);
ssize_t parseLabel(const Str *label);

typedef enum {
  UXTB,
  UXTH,
  UXTW,
  LSL,
  UXTX,
  SXTB,
  SXTH,
  SXTW,
  SXTX,
} ExtendType;

typedef struct {
  u8 imm;
  ExtendType t;
} Extend;

static Extend parseExtend(const Str *label, const Str *amount) {
  return (Extend){0};
}

Argument getType(const Str *s);
#endif
