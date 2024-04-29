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
  ShiftType sh_t;
  if (strcmp(shift, "lsl") == 0) {
    sh_t = SH_LSL;
  } else if (strcmp(shift, "lsr") == 0) {
    sh_t = SH_LSR;
  } else if (strcmp(shift, "asr") == 0) {
    sh_t = SH_ASR;
  } else if (strcmp(shift, "ror") == 0) {
    sh_t = SH_ROR;
  } else {
    return 0;
  }

  if (sh) {
    *sh = sh_t;
  }
  return 1;
}

u8 parseExtend(const char *extend, ExtendType *ex) {
  ExtendType ext = 0;

  if (strcmp(extend, "lsl") == 0) {
    ext = LSL;
  } else if (strcmp(extend, "uxtb") == 0) {
    ext = UXTB;
  } else if (strcmp(extend, "uxth") == 0) {
    ext = UXTH;
  } else if (strcmp(extend, "uxtw") == 0) {
    ext = UXTW;
  } else if (strcmp(extend, "uxtx") == 0) {
    ext = UXTX;
  } else if (strcmp(extend, "sxtb") == 0) {
    ext = SXTB;
  } else if (strcmp(extend, "sxth") == 0) {
    ext = SXTH;
  } else if (strcmp(extend, "sxtw") == 0) {
    ext = SXTW;
  } else if (strcmp(extend, "sxtx") == 0) {
    ext = SXTX;
  } else {
    return 0;
  }

  if (ex) {
    *ex = ext;
  }
  return 1;
}
