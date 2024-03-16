#include "parser.h"
#include "../common/erorrs.h"
#include "../tables/tables.h"
#include "../instructions/instructions.h"
#include <ctype.h>

void discardComment(char *buffer) {
  while (*buffer) {
    if (*buffer == '/' && *(buffer + 1) == '/') {
      *buffer = '\0';
      return;
    }
    buffer++;
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
  *(line + 1) = '\0';
}

void preprocessLine(char *line) {
  discardComment(line);
  trim(line);
}

void toLower(char *line) {
  while (*line) {
    *line = tolower(*line);
    line++;
  }
}

// u8 splitLine(const char *line, fields_t *fields) {
//   fields->count = 0;
//   u8 c = 0;
// 
//   while (*line) {
//     switch (*line) {
//     case '\n':
//       return fields->count;
//     case ' ':
//     case ',':
//     case '\t':
//       if (c == 0) {
//         break;
//       }
//       fields->count++;
//       c = 0;
//       break;
//     default:
//       fields->fields[fields->count][c] = *line;
//       c++;
//       break;
//     }
//     line++;
//   }
//   return fields->count;
// }

Register parseRegister(const char *reg) {
  if (strcmp(reg, "xzr") == 0) {
    return REG_ZR;
  }
  if (strcmp(reg, "sp") == 0) {
    return REG_SP;
  }

  char *end = NULL;
  long n = strtol(reg + 1, &end, 10);
  if (n < 0 || n > 30 || end) {
    return REG_NONE;
  }
  return REG_GENERAL;
}

u8 parseImmediate(const char *immediate, u32 *n) {
  if (*immediate == '#') {
    immediate++;
  }

  u8 base = 10;
  char *end = 0;
  if (*(immediate + 1) == 'x' && strlen(immediate) > 2) {
    base = 16;
    immediate += 2;
  }
  *n = strtol(immediate, &end, base);
  if (end) {
    return 0;
  }
  return 1;
}

int firstPass(FILE *src) {
  if (!src) {
    return -1;
  }

  char buffer[LINE_MAX] = {0};
  while (fgets(buffer, sizeof(buffer), src)) {
    preprocessLine(buffer);
    if (buffer[strlen(buffer) - 1] == ':') {
    }
  }

  fseek(src, 0, SEEK_SET);
  return 0;
}
