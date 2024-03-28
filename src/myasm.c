#include "myasm.h"
#include "tables/tables.h"
#include <ctype.h>

void *xmalloc(size_t size) {
  void *p = malloc(size);
  if (!p) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  return p;
}

void *xrealloc(void *p, size_t size) {
  p = realloc(p, size);
  if (!p) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  return p;
}

void firstPass(const char *filename, FILE *ir_file);
void secondPass(FILE *ir_file, FILE *dst);

int main(int argc, char **argv) {
  if (argc == 1) {
    fprintf(stderr, "Please, provide source file");
    return EXIT_SUCCESS;
  }

  // ir_file
  // TYPE|VALUE|PC
  FILE *ir_file = tmpfile();
  if (!ir_file) {
    fprintf(stderr, "Failed to open ir file. Error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  FILE *dst = fopen("a.out", "w");
  if (!dst) {
    fprintf(stderr, "Failed to open out. Error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  firstPass(argv[1], ir_file);
  secondPass(ir_file, dst);

  fclose(ir_file);
  fclose(dst);
  return EXIT_SUCCESS;
}

typedef enum {
  INSTRUCTION,
  DIRECTIVE,
  LABEL,
} LineType;

void discardComment(char *buffer);
void trim(char *line);
void preprocessLine(char *line);
LineType getType(char *line);
u8 splitLine(const char *line, fields_t *fields);

// DIRECTIVES
void global_d(fields_t *fields) {}
void section_d(fields_t *fields) {}

typedef struct Directive {
  const Str name;
  void (*f)(fields_t *fields);
} Directive;

Directive DIRECTIVES[] = {
    {{"global", strlen("global"), strlen("global")}, &global_d},
    {{"section", strlen("section"), strlen("section")}, &global_d},
};

// INSTRUCTIONS

#define ARM_INSTRUCTION_SIZE (4)
#define LINE_MAX UINT8_MAX
#define ELF64_SIZE (64)

void firstPass(const char *filename, FILE *ir_file) {
  FILE *src = fopen(filename, "r");
  if (!src) {
    fprintf(stderr, "Failed to open src file. Error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  initTables();

  size_t pc = 0;
  fields_t fields = initFields();
  char buf[LINE_MAX] = {0};

  while (fgets(buf, LINE_MAX, src)) {
    preprocessLine(buf);
    if (!*buf) {
      continue;
    }
    splitLine(buf, &fields);

    LineType type = 0;
    switch (type = getType(fields.fields[0].s)) {
    case DIRECTIVE:
    case INSTRUCTION:
      fprintf(ir_file, "%d|%s|%lu\n", type, buf, pc);
      if (type == INSTRUCTION) {
        pc += ARM_INSTRUCTION_SIZE;
      }
      break;

    case LABEL:
      addToSym(fields.fields[0].s, pc, 0, ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE));
      break;
    }
  }

  freeFields(&fields);
  fseek(ir_file, 0, SEEK_SET);
  fclose(src);
}

void secondPass(FILE *ir_file, FILE *dst) {
  char buf[LINE_MAX] = {0};
  LineType type = 0;
  size_t pc = 0;

  fseek(dst, ELF64_SIZE, SEEK_SET); // skip first 64 bytes for elf header

  while (fscanf(ir_file, "%d|%s|%lu\n", &type, buf, &pc) != EOF) {
    switch (type) {
    case INSTRUCTION:
      break;
    case DIRECTIVE:
      break;
    }
  }
}

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
  *line = '\0';
}

void preprocessLine(char *line) {
  discardComment(line);
  trim(line);
}

LineType getType(char *line) {
  if (line[strlen(line) - 1] == ':') {
    line[strlen(line) - 1] = '\0';
    return LABEL;
  }

  if (*line == '.') {
    return DIRECTIVE;
  }

  return INSTRUCTION;
}

u8 splitLine(const char *line, fields_t *fields) {
  fields->n_fields = 0;
  for (u8 i = 0; i < FIELDS_MAX; i++) {
    fields->fields[i].len = 0;
  }

  size_t *i = &fields->fields[fields->n_fields].len;
  do {
    switch (*line) {
    case '\n':
    case ' ':
    case ',':
      if (*i == 0) { // in case ", " so , - skipped and i == 0 so skip " " too
        break;
      }
      fields->fields[fields->n_fields].s[*i] = '\0';
      fields->n_fields++;
      i = &fields->fields[fields->n_fields].len;
      break;
    default:
      fields->fields[fields->n_fields].s[*i] = *line;
      *i += 1;
      break;
    }
  } while (*line++);

  return fields->n_fields;
}
