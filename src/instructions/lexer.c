#include "lexer.h"
#include "assemble.h"
#include "directives/directives.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

FILE *SRC = NULL;

// maybe add some constraints on label naming?
u8 isLabelDeclaration(const char *label) {
  if (label[strlen(label) - 1] == ':') {
    return 1;
  }
  return 0;
}

TokenType getTokenType(Token *t) {
  switch (*t->value) {
  case '+':
    return T_PLUS;
  case '-':
    return T_MINUS;
  case '[':
    return T_RSBRACE;
  case ']':
    return T_LSBRACE;
  case '!':
    return T_BANG;
  case '$':
    return T_DOLLAR;
  }

  if (parseRegister(t->value, NULL)) {
    return T_REGISTER;
  }

  if (parseImmediateU64(t->value, NULL)) {
    return T_IMMEDIATE;
  }

  if (parseExtend(t->value, NULL)) {
    return T_EXTEND;
  }

  if (parseShift(t->value, NULL)) {
    return T_SHIFT;
  }

  if (searchMnemonic(t->value)) {
    return T_INSTRUCTION;
  }

  if (searchDirective(t->value + 1)) {
    return T_DIRECTIVE;
  }

  if (isLabelDeclaration(t->value)) {
    return T_LABEL_DECLARATION;
  }

  return T_LABEL;
}

u8 getToken(FILE *f, Token *t) {
  if (f) {
    SRC = f;
  }

  if ((!SRC && !f) || !t) {
    return 0;
  }

  char ch;
  size_t i = 0;
  // you cant use upper case for labels
  while ((ch = toupper(getc(SRC))) != EOF) {
    switch (ch) {
    case '/':
      do {
        ch = fgetc(SRC);
      } while (ch != '\n' && ch != EOF);
      __attribute__((fallthrough));
    case ' ':
    case ',':
    case '\t':
    case '\n':
      // skip the char
      if (i != 0) {
        goto done;
      }
      break;
    case '+':
    case '-':
    case '[':
    case ']':
    case '!':
    case '$':
      if (i != 0) {
        fseek(SRC, -1, SEEK_CUR);
        goto done;
      }
      t->value[i] = ch;
      i++;
      goto done;
    default:
      if (i == t->capacity) {
        goto done;
      }
      t->value[i] = ch;
      i++;
      break;
    }
  }

  if (ch == EOF) {
    return 0;
  }

done:
  t->value[i] = '\0';
  t->type = getTokenType(t);
  return 1;
}

u8 parseDigit(const char *number, u64 *res) {
  if (!number) {
    return 0;
  }

  u8 base = 10;
  const char *d = number;
  if (*d == '0' && *(d + 1) == 'X') {
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
  if (*reg != 'W' && *reg != 'X' && *reg != 'S') {
    return 0;
  }

  u8 n = 0;
  if (strcmp(reg + 1, "ZR") == 0) {
    n = REGISTER_ZR_SP;
  }

  if (strcmp(reg, "WSP") == 0 || strcmp(reg, "SP") == 0) {
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
    r->extended = *reg == 'X' || *reg == 'S';
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
  if (strcmp(shift, "LSL") == 0) {
    sh_t = SH_LSL;
  } else if (strcmp(shift, "LSR") == 0) {
    sh_t = SH_LSR;
  } else if (strcmp(shift, "ASR") == 0) {
    sh_t = SH_ASR;
  } else if (strcmp(shift, "ROR") == 0) {
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

  if (strcmp(extend, "LSL") == 0) {
    ext = LSL;
  } else if (strcmp(extend, "UXTB") == 0) {
    ext = UXTB;
  } else if (strcmp(extend, "UXTH") == 0) {
    ext = UXTH;
  } else if (strcmp(extend, "UXTW") == 0) {
    ext = UXTW;
  } else if (strcmp(extend, "UXTX") == 0) {
    ext = UXTX;
  } else if (strcmp(extend, "SXTB") == 0) {
    ext = SXTB;
  } else if (strcmp(extend, "SXTH") == 0) {
    ext = SXTH;
  } else if (strcmp(extend, "SXTW") == 0) {
    ext = SXTW;
  } else if (strcmp(extend, "SXTX") == 0) {
    ext = SXTX;
  } else {
    return 0;
  }

  if (ex) {
    *ex = ext;
  }
  return 1;
}

Token initToken(size_t size) {
  Token t = {0};
  t.capacity = size;
  t.value = xmalloc(t.capacity * sizeof(*t.value));
  return t;
}

Fields initFields(size_t size_of_field) {
  Fields f = {0};
  for (u8 i = 0; i < FIELDS_MAX; i++) {
    f.fields[i] = initToken(size_of_field);
  }
  return f;
}

void copyToken(Token *dst, Token *src) {
  if (dst->capacity < src->capacity) {
    dst->capacity = src->capacity;
    dst->value = xrealloc(dst->value, dst->capacity * sizeof(*dst->value));
  }
  dst->type = src->type;
  strcpy(dst->value, src->value);
}

void freeFields(Fields *fields) {
  if (!fields) {
    return;
  }
  for (u8 i = 0; i < FIELDS_MAX; i++) {
    free(fields->fields[i].value);
  }
}

u8 writeToken(FILE *f, Token *t) {
  if (!f || !t) {
    return 0;
  }

  fwrite(&t->type, sizeof(t->type), 1, f);
  fwrite(&t->capacity, sizeof(t->capacity), 1, f);

  size_t n = strlen(t->value) + 1;
  fwrite(&n, sizeof(n), 1, f);
  fwrite(t->value, n, 1, f);
  return 1;
}

u8 readToken(FILE *f, Token *t) {
  if (!f || !t) {
    return 0;
  }

  if (feof(f)) {
    return 0;
  }

  fread(&t->type, sizeof(t->type), 1, f);
  fread(&t->capacity, sizeof(t->capacity), 1, f);

  size_t n;
  fread(&n, sizeof(n), 1, f);
  fread(t->value, n, 1, f);
  return 1;
}
