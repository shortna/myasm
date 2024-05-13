#include "lexer.h"
#include "directives.h"
#include "instructions.h"
#include "parser.h"
#include "types.h"
#include <ctype.h>
#include <stdio.h>

FILE *SRC = NULL;
size_t LINE = 0;

TokenType getTokenType(Token *t) {
  switch (*t->value) {
  case '[':
    return T_RSBRACE;
  case ']':
    return T_LSBRACE;
  case '!':
    return T_BANG;
  case '=':
    return T_EQUAL;
  }

  if (parseRegister(t->value, NULL)) {
    return T_REGISTER;
  }

  if (parseImmediateU64(t->value, NULL)) {
    return T_IMMEDIATE;
  }

  if (searchMnemonic(t->value)) {
    return T_INSTRUCTION;
  }

  if (searchDirective(t->value + 1) != D_NONE) {
    return T_DIRECTIVE;
  }

  if (parseLabelDeclaration(t->value)) {
    return T_LABEL_DECLARATION;
  }

  if (parseCondition(t->value, NULL)) {
    return T_CONDITION;
  }

  if (parseExtend(t->value, NULL)) {
    return T_EXTEND;
  }

  if (parseShift(t->value, NULL)) {
    return T_SHIFT;
  }

  if (isString(t->value)) {
    return T_STRING;
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

  size_t i = 0;
  int ch;
  while ((ch = tolower(getc(SRC))) != EOF) {
    switch (ch) {
    case '/':
      while (ch != '\n' && ch != EOF) {
        ch = fgetc(SRC);
      }
      __attribute__((fallthrough));
    case '\n':
      LINE++;
      __attribute__((fallthrough));
    case ' ':
    case ',':
    case '\t':
      if (i != 0) {
        goto done;
      }
      break;
    case '"':
      do {
        t->value[i++] = ch;
        ch = fgetc(SRC);
        if (i == t->capacity - 2) {
          resizeToken(t); 
        }
      } while (ch != '"' && ch != EOF);
      t->value[i++] = ch;
      break;
    case '[':
    case ']':
    case '!':
    case '=':
      if (i != 0) {
        fseek(SRC, -1, SEEK_CUR);
        goto done;
      }
      t->value[i++] = ch;
      goto done;
    case '.':
      if (i != 0) {
        t->value[i++] = ch;
        goto done;
      }
      __attribute__((fallthrough));
    default:
      if (i == t->capacity) {
        goto done;
      }
      t->value[i++] = ch;
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
