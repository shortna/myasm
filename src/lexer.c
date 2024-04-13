#include "lexer.h"
#include "myasm.h"
#include <ctype.h>
#include <string.h>

FILE *SRC = NULL;

u8 getToken(FILE *f, Token *t) {
  if ((!SRC && !f) || !t) {
    return 0;
  }
  if (f) {
    SRC = f;
  }
  t->type = T_NONE;

  int ch;
  while ((ch = getc(f)) != EOF) {
    switch (ch) {
    case '+':
    case '-':
    case '[':
    case ']':
    case '!':
    case '$':
      t->type = ;
      *t->value = ch;
      goto done;
    }
  }

done:
  *(t->value + 3) = '\0';
  return 1;
}

void discardComment(char *line) {
  while (*line) {
    if (*line == '/' && *(line + 1) == '/') {
      *line = '\0';
      return;
    }
    line++;
  }
}

void trim(char *line) {
  char *begin = line;
  char *end = begin + strlen(begin) - 1;
  while (isspace(*begin)) {
    begin++;
  }

  if (*begin == '\0') {
    *line = '\0';
    return;
  }

  while (isspace(*end) && end >= begin) {
    end--;
  }

  for (char *i = begin; i <= end; i++) {
    *line = *i;
    line++;
  }
  *line = '\0';
}
