#include "parse.h"

Argument getType(const Str *s) {
  const char *str = s->s;
  if (strcmp(str, "SP") == 0 || strcmp(str, "WSP") == 0) {
    return SP;
  } 

  if (*str == '#') {
    for (size_t i = 0; i < s->len; i++) {
    }
  }

  return NO_ARG;
}

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

  long res = strtol(reg->s + 1, NULL, 10);
  if (res >= 0 || res <= 29) {
    if (*reg->s == 'X') {
      return (Register){true, res};
    } else {
      return (Register){false, res};
    }
  }

  return (Register){false, -1};
}

u8 parseImmediate(const Str *imm, i64 *res) {
  const char *s = imm->s;
  if (*s == '#') {
    s++;
  }

  char *end = NULL;
  *res = 0;

  if (*(s + 1) == 'x') {
    *res = strtol(s, &end, 16);
  } else {
    *res = strtol(s, &end, 10);
  }

  if (end == imm->s + imm->len) {
    return 1;
  }

  return 0;
}

ssize_t parseLabel(const Str *label) { return getLabelPc(label->s); }

u8 parseShift(const Str *shift_type, const Str *amount, Shift *sh) {
  if (shift_type == NULL || amount == NULL) {
    sh->imm = 0;
    sh->t = SH_LSL;
    return 1;
  }

  if (strcmp(shift_type->s, "lsl") == 0) {
    sh->t = SH_LSL;
  } else if (strcmp(shift_type->s, "lsr") == 0) {
    sh->t = SH_LSR;
  } else if (strcmp(shift_type->s, "asr") == 0) {
    sh->t = SH_ASR;
  } else if (strcmp(shift_type->s, "ror") == 0) {
    sh->t = SH_ROR;
  } else {
    return 0;
  }

  if (!parseImmediate(amount, &sh->imm)) {
    return 0;
  }

  return 1;
}
