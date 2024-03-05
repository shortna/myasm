#include "lexer.h"
#include <ctype.h>

static FILE *SRC_FILE = NULL;

TokenType getTokenType(const Token *token) {
  if (!token) {
    return NONE;
  }

  const char *value = token->value;

  if (value[token->len - 1] == ':') {
    while (*value != ':') {
      if (islower(*value) || *value == '_') {
        value++;
        continue;
      }
      return NONE;
    }
    return LABEL_DECLARATION;
  }

  switch (*value) {
  case '!':
    return BANG;
  case '[':
    return R_SBRACE;
  case ']':
    return L_SBRACE;
  case '=':
    return EQUAL;
  case '-':
    return MINUS;
  case '+':
    return PLUS;
  case '*':
    return STAR;
  case '"':
    return STRING;

  case '.':
    if (*(value + 1) == '\0') {
      return DOT;
    }
    return DIRECTIVE;

  case '#': {
    char ch = 0;
    value++;
    switch (*(value + 1)) {
    case 'x':
      sscanf(value, "%*x%c", &ch);
      break;
    case 'b':
      sscanf(value, "%*255[01]%c", &ch);
      break;
    default:
      sscanf(value, "%*d%c", &ch);
      break;
    }

    if (!ch) {
      return NUMBER;
    }
    return NONE;
  }

  case 's':
  case 'l':
  case 'x':
  case 'w':
    if (strcmp(value, "sp") == 0 || strcmp(value, "lr") == 0 ||
        strcmp(value, "xzr") == 0 || strcmp(value, "wzr") == 0) {
      return REGISTER;
    }

    value++;

    char ch = 0;
    int n = 0;
    sscanf(value, "%i%c", &n, &ch);

    if (n >= 0 && n <= 30 && !ch) {
      return REGISTER;
    }

    __attribute__((fallthrough));

  default:
    return INSTRUCTION_OR_LABEL;
  }
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
    if (i == MAX_TOKEN_SIZE) {
      goto end;
    }

    switch (ch) {
    case -1:
      return -1;

    case '\n':
    case ' ':
    case ',':
    case '\t':
      if (*token->value != '\0') {
        goto end;
      }
      break;

    case '/':
      ch = fgetc(SRC_FILE);
      if (ch != '/') {
        fseek(SRC_FILE, -1, SEEK_CUR);
        token->value[i] = ch;
        i++;
        goto end;
      }

      while (ch != '\n') {
        ch = fgetc(SRC_FILE);
      }
      break;

    case '"':
      do {
        token->value[i] = ch;
        ch = fgetc(SRC_FILE);
        i++;
      } while (ch != '"');
      token->value[i] = ch;
      i++;
      break;

    case '=':
    case '[':
    case ']':
    case '+':
    case '-':
    case '*':
    case '!':
      if (*token->value != '\0') {
        fseek(SRC_FILE, -1, SEEK_CUR);
        goto end;
      }
      token->value[i] = ch;
      i++;
      goto end;

    default:
      token->value[i] = tolower(ch);
      i++;
    }
  }

end:
  token->value[i] = '\0';
  token->len = strlen(token->value);
  token->type = getTokenType(token);

  return 1;
}
