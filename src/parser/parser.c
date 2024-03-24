#include "parser.h"
#include "../common/erorrs.h"
#include "../instructions/instructions.h"
#include "../tables/tables.h"
#include <ctype.h>

typedef struct {
  size_t count;
  char fields[FIELDS_MAX][NAME_MAX];
} fields_t;

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

void toUpper(char *line) {
  while (*line) {
    *line = toupper(*line);
    line++;
  }
}

u8 splitLine(const char *line, fields_t *fields) {
  fields->count = 0;
  u8 i = 0;

  while (*line) {
    switch (*line) {
    case '\n':
      return fields->count;
    case ' ':
    case ',':
    case '\t':
      fields->count++;
      i = 0;
      break;
    default:
      fields->fields[fields->count][i] = *line;
      i++;
      break;
    }
    line++;
  }
  return fields->count;
}

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

u8 parseLabel(const char *label) {
  if (*(label + strlen(label) - 1) != ':') {
    return 0;
  }

  while (*label) {
    if (!isalpha(*label) && !isdigit(*label)) {
      return 0;
    }
    label++;
  }

  return 1;
}

u8 parseDirective(const char *label) {
  if (*label == '.') {
  }

  return 1;
}

int firstPass(FILE *src) {
  if (!src) {
    return -1;
  }
  initTables();
  size_t pc = 0;

  char buffer[LINE_MAX] = {0};
  fields_t fields = {0};

  while (fgets(buffer, sizeof(buffer), src)) {
    preprocessLine(buffer);
    if (parseLabel(buffer)) {
      addToSym(buffer, pc, 0, ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE));
    } else {
      splitLine(buffer, &fields);
      if (searchInstruction(fields.fields[0])) {
        pc += INSTRUCTION_SIZE;
      }
    }
  }

  fseek(src, 0, SEEK_SET);
  return 0;
}
