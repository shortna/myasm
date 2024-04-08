#include "parse.h"
#include "tables/tables.h"
#include <stdlib.h>
#include <string.h>

Register parseRegister(const Str *reg) {
  if (*reg->s == 'W') {
    if (strcmp(reg->s, "WZR") == 0) {
      return (Register){false, 31};
    }
    if (strcmp(reg->s, "WSP") == 0) {
      return (Register){false, 30};
    }
  } else {
    if (strcmp(reg->s, "XZR") == 0) {
      return (Register){true, 31};
    }

    if (strcmp(reg->s, "SP") == 0) {
      return (Register){true, 30};
    }
  }

  char *end = NULL;
  long res = strtol(reg->s + 1, &end, 10);

  if (end != reg->s + reg->len) {
    return (Register){false, -1};
  }

  if (res >= 0 || res <= 29) {
    if (*reg->s == 'X') {
      return (Register){true, res};
    } else {
      return (Register){false, res};
    }
  }

  return (Register){false, -1};
}

ssize_t parseLabel(const Str *label) { return getLabelPc(label->s); }

u8 parseShift(const Str *shift_type, const Str *amount, Shift *sh) {
  if (shift_type == NULL && amount == NULL) {
    sh->imm = 0;
    sh->t = SH_LSL;
    return 1;
  }

  if (strcmp(shift_type->s, "LSL") == 0) {
    sh->t = SH_LSL;
  } else if (strcmp(shift_type->s, "LSR") == 0) {
    sh->t = SH_LSR;
  } else if (strcmp(shift_type->s, "ASR") == 0) {
    sh->t = SH_ASR;
  } else if (strcmp(shift_type->s, "ROR") == 0) {
    sh->t = SH_ROR;
  } else {
    return 0;
  }

  if (!parseImmediateU8(amount, &sh->imm)) {
    return 0;
  }

  return 1;
}

i64 parseDigit(const Str *digit) {
  if (!digit) {
    return -1;
  }

  size_t i = 0;
  if (*digit->s == '#') {
    i++;
  }

  u8 base = 10;
  if (*(digit->s + i + 1) == 'x') {
    base = 16;
  }

  char *end = NULL;
  i64 res = strtoll(digit->s, &end, base);
  if (end == digit->s + digit->len) {
    return res;
  }

  return -1;
}

u8 parseImmediateU8(const Str *imm, u8 *res) {
  i64 n = parseDigit(imm);
  if (n == -1) {
    return 0;
  }

  if ((u64)n > UINT8_MAX) {
    return 0;
  }

  *res = n;
  return 1;
}

u8 parseImmediateU16(const Str *imm, u16 *res) {
  i64 n = parseDigit(imm);
  if (n == -1) {
    return 0;
  }

  if ((u64)n > UINT16_MAX) {
    return 0;
  }

  *res = n;
  return 1;
}

u8 parseImmediateU32(const Str *imm, u32 *res) {
  i64 n = parseDigit(imm);
  if (n == -1) {
    return 0;
  }

  if ((u64)n > UINT32_MAX) {
    return 0;
  }

  *res = n;
  return 1;
}

u8 parseImmediateU64(const Str *imm, u64 *res) {
  i64 n = parseDigit(imm);
  if (n == -1) {
    return 0;
  }

  if ((u64)n > UINT32_MAX) {
    return 0;
  }

  *res = n;
  return 1;
}

Argument getArgumentType(const Str *s) {
  // register
  i64 n;
  if ((n = parseRegister(s).n) != -1) {
    if (n == 30) {
      return SP;
    }
    return REGISTER;
  }

  // immediate
  if (parseDigit(s) != -1) {
    return IMMEDIATE;
  }

  // label
  if (parseLabel(s) != -1) {
    return LABEL;
  }

  // shift
  if (strcmp(s->s, "lsl") == 0 || strcmp(s->s, "lsr") == 0 ||
      strcmp(s->s, "asr") == 0 || strcmp(s->s, "ror") == 0) {
    return SHIFT;
  }

  return NO_ARG;
}
