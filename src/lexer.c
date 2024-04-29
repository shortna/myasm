#include "lexer.h"
#include "directives.h"
#include "instructions_api.h"
#include "parser.h"
#include "types.h"
#include <ctype.h>
#include <stdio.h>

FILE *SRC = NULL;
size_t LINE = 0;

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

  if (searchDirective(t->value + 1) != -1) {
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
  while ((ch = tolower(getc(SRC))) != EOF) {
    switch (ch) {
    case '\n':
      LINE++;
      __attribute__((fallthrough));
    case '/':
      while (ch != '\n' && ch != EOF) {
        ch = fgetc(SRC);
      }
      __attribute__((fallthrough));
    case ' ':
    case ',':
    case '\t':
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
    LINE = 0;
    return 0;
  }

done:
  t->value[i] = '\0';
  t->type = getTokenType(t);
  return 1;
}
