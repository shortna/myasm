#include "lexer.h"
#include "instructions_api.h"
#include "directives.h"
#include "types.h"
#include "parser.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *SRC = NULL;

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

  if (parseLabelDeclaration(t->value)) {
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

void copyToken(Token *dst, const Token *src) {
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

u8 writeToken(FILE *f, const Token *t) {
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
