#include "lexer.h"
// #include "../assemble.h"
// #include "./directives/directives.h"
#include <ctype.h>
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
    return T_LSBRACE;
  case ']':
    return T_RSBRACE;
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

  //  if (searchDirective()) {
  //  }
  //
  //  if (searchInstruction()) {
  //  }

  if (isLabelDeclaration(t->value)) {
    return T_LABEL_DECLARATION;
  }

  return T_LABEL;
}

u8 getToken(FILE *f, Token *t) {
  if ((!SRC && !f) || !t) {
    return 0;
  }
  if (f) {
    SRC = f;
  }

  char ch;
  size_t i = 0;
  // you cant use upper case for labels
  while ((ch = tolower(getc(f))) != EOF) {
    switch (ch) {
    case '/':
      do {
        ch = fgetc(f);
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
        fseek(f, -1, SEEK_CUR);
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

done:
  t->value[i] = '\0';
  t->type = getTokenType(t);
  return 1;
}

u8 parseDigit(const char *digit, i64 *res) {}

u8 parseRegister(const char *reg, Register *r) {
  if (*reg != 'W' && *reg != 'X' && *reg != 'S') {
    return 0;
  }

  u8 n = 0;
  if (strcmp(reg + 1, "ZR") == 0) {
    n = 31;
  }

  if (strcmp(reg, "WSP") == 0 || strcmp(reg, "SP") == 0) {
    n = 30;
  }

  i64 res = -1;
  if (!parseDigit(reg + 1, &res)) {
    return 0;
  }

  if (res >= 0 && res <= 29) {
    n = res;
  }

  if (r) {
    r->extended = *reg == 'X';
    r->n = res;
  }
  return 1;
}

u8 parseImmediateU8(const char *imm, u8 *res) {
  i64 n;
  if (!parseDigit(imm, &n)) {
    return 0;
  }

  if ((u64)n > UINT8_MAX) {
    return 0;
  }

  *res = n;
  return 1;
}

u8 parseImmediateU16(const char *imm, u16 *res) {
  i64 n;
  if (!parseDigit(imm, &n)) {
    return 0;
  }

  if ((u64)n > UINT16_MAX) {
    return 0;
  }

  *res = n;
  return 1;
}

u8 parseImmediateU32(const char *imm, u32 *res) {
  i64 n;
  if (!parseDigit(imm, &n)) {
    return 0;
  }

  if ((u64)n > UINT32_MAX) {
    return 0;
  }

  *res = n;
  return 1;
}

u8 parseImmediateU64(const char *imm, u64 *res) {
  i64 n;
  if (!parseDigit(imm, &n)) {
    return 0;
  }

  *res = n;
  return 1;
}

u8 parseShift(const char *shift, ShiftType *sh) {
  if (strcmp(shift, "LSL") == 0) {
    *sh = SH_LSL;
  } else if (strcmp(shift, "LSR") == 0) {
    *sh = SH_LSR;
  } else if (strcmp(shift, "ASR") == 0) {
    *sh = SH_ASR;
  } else if (strcmp(shift, "ROR") == 0) {
    *sh = SH_ROR;
  } else {
    return 0;
  }

  return 1;
}

u8 parseExtend(const char *extend, ExtendType *ex) {
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

void freeFields(Fields *fields) {
  if (!fields) {
    return;
  }
  for (u8 i = 0; i < FIELDS_MAX; i++) {
    free(fields->fields[i].value);
  }
}
