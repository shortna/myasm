#include "parser.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// maybe add some constraints on label naming?
u8 parseLabelDeclaration(char *label) {
  if (label[strlen(label) - 1] == ':') {
    label[strlen(label) - 1] = '\0';
    return 1;
  }
  return 0;
}

u8 parseDigit(const char *number, u64 *res) {
  if (!number) {
    return 0;
  }

  u8 base = 10;
  const char *d = number;
  if (*d == '-' || *d == '+') {
    d++;
  }
  if (*d == '0' && *(d + 1) == 'x') {
    base = 0;
  }

  errno = 0;
  char *end = NULL;
  *res = strtoull(number, &end, base);
  if (errno != 0 || *end != '\0') {
    return 0;
  }

  return 1;
}

u8 parseRegister(const char *reg, Register *r) {
  if (*reg != 'w' && *reg != 'x' && *reg != 's') {
    return 0;
  }

  u8 n = 0;
  if (strcmp(reg + 1, "zr") == 0) {
    n = REGISTER_ZR_SP;
  }

  if (strcmp(reg, "wsp") == 0 || strcmp(reg, "sp") == 0) {
    n = REGISTER_ZR_SP;
  }

  u64 res;
  if (n == 0) {
    if (!parseDigit(reg + 1, &res)) {
      return 0;
    }

    if (res >= 0 && res <= 29) {
      n = res;
    }
  }

  if (r) {
    r->extended = *reg == 'x' || *reg == 's';
    r->n = n;
  }
  return 1;
}

u8 parseImmediateI64(const char *imm, i64 *res) {
  u64 n;

  if (*imm == '#') {
    imm++;
  }

  if (!parseDigit(imm, &n)) {
    return 0;
  }

  if (res) {
    *res = n;
  }
  return 1;
}

u8 parseImmediateU8(const char *imm, u8 *res) {
  u64 n;

  if (*imm == '#') {
    imm++;
  }

  if (!parseDigit(imm, &n)) {
    return 0;
  }

  if ((u64)n > UINT8_MAX) {
    return 0;
  }

  if (res) {
    *res = n;
  }
  return 1;
}

u8 parseImmediateU16(const char *imm, u16 *res) {
  u64 n;

  if (*imm == '#') {
    imm++;
  }

  if (!parseDigit(imm, &n)) {
    return 0;
  }

  if ((u64)n > UINT16_MAX) {
    return 0;
  }

  if (res) {
    *res = n;
  }
  return 1;
}

u8 parseImmediateU32(const char *imm, u32 *res) {
  u64 n;

  if (*imm == '#') {
    imm++;
  }

  if (!parseDigit(imm, &n)) {
    return 0;
  }

  if ((u64)n > UINT32_MAX) {
    return 0;
  }

  if (res) {
    *res = n;
  }
  return 1;
}

u8 parseImmediateU64(const char *imm, u64 *res) {
  u64 n;

  if (*imm == '#') {
    imm++;
  }

  if (!parseDigit(imm, &n)) {
    return 0;
  }

  if (res) {
    *res = n;
  }
  return 1;
}

u8 parseShift(const char *shift, ShiftType *sh) {
  ShiftType sh_t = -1;

  for (size_t i = 0; i < sizeof(SHIFTS) / sizeof(*SHIFTS); i++) {
    if (strcmp(shift, SHIFTS[i].value) == 0) {
      sh_t = SHIFTS[i].type;
    }
  }

  // dirty hack
  if (sh_t == (ShiftType)-1) {
    return 0;
  }

  if (sh) {
    *sh = sh_t;
  }
  return 1;
}

u8 parseExtend(const char *extend, ExtendType *ex) {
  ExtendType ex_t;

  for (size_t i = 0; i < sizeof(EXTENDS) / sizeof(*EXTENDS); i++) {
    if (strcmp(extend, EXTENDS[i].value) == 0) {
      ex_t = EXTENDS[i].type;
    }
  }

  // dirty hack
  if (ex_t == (ExtendType)-1) {
    return 0;
  }

  if (ex) {
    *ex = ex_t;
  }
  return 1;
}

u8 parseCondition(const char *condition, ConditionType *c) {
  ConditionType c_t = -1;

  for (size_t i = 0; i < sizeof(CONDITIONS) / sizeof(*CONDITIONS); i++) {
    if (strcmp(condition, CONDITIONS[i].value) == 0) {
      c_t = CONDITIONS[i].type;
    }
  }

  // dirty hack
  if (c_t == (ConditionType)-1) {
    return 0;
  }

  if (c) {
    *c = c_t;
  }
  return 1;
}

u8 isString(const char *str) {
  if (*str == '"' && str[strlen(str) - 1] == '"') {
    return 1;
  }
  return 0;
}
