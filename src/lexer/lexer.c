#include "lexer.h"
#include <ctype.h>
#include <string.h>

static FILE *SRC_FILE = NULL;
extern DLA sym_table;
extern char *ISA;
extern size_t ISA_LEN;

TokenType getTokenType(const Token *token) {
  if (!token) {
    return NONE;
  }
  const char *value = token->value;

  if (value[token->len - 1] == ':') {
    while (*value != ':') {
      if (islower(*value) || isupper(*value) || *value == '_') {
        value++;
        continue;
      }
      return NONE;
    }
    return LABEL_DECLARATION;
  }

  size_t i = 0, j = 0;
  while (i < ISA_LEN) {
    j = 0;
    while (ISA[i + j] != '\n') {
      j++;
    }
    j++;
    if (strncmp(&ISA[i], token->value, j - 1) == 0) {
      return INSTRUCTION;
    }
    i += j;
  }

  switch (*value) {
  case '.':
    if (*(value + 1) == '\0') {
      return LABEL;
    }
    return DIRECTIVE;
  case '#':
    return NUMBER;
  case '"':
    return ARGUMENT;
  case 'X':
  case 'W':
    return REGISTER;
  }

  return NONE;
}

i8 getToken(FILE *src, Token *token) {
  if (!src && !SRC_FILE) {
    return 0;
  }

  if (src) {
    SRC_FILE = src;
  }

  if (!token) {
    return 0;
  }

  *token->value = '\0';
  u8 i = 0;

  while (1) {
    char ch = fgetc(SRC_FILE);
    if (ch == -1) {
      return -1;
    }
    if (i == MAX_TOKEN_SIZE) {
      goto end;
    }

    switch (ch) {
    case ' ':
    case '\n':
    case '\t':
    case ',':
      if (*token->value != '\0') {
        goto end;
      }
      break;

    case '/':
      while (ch != '\n') {
        ch = fgetc(SRC_FILE);
      }
      break;

    case '"':
      do {
        token->value[i] = ch;
        ch = fgetc(SRC_FILE);
        i++;
      } while (ch != '\n');
      break;

    default:
      token->value[i] = ch;
      i++;
    }
  }

end:
  token->len = strlen(token->value);
  token->type = 0;
  token->type = getTokenType(token);

  if (token->type == LABEL_DECLARATION) {
    AddLabel(token->value);
  }
  return 1;
}
