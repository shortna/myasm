#include "myasm.h"
#include "tables/tables.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #ifdef __aarch64__
#include "instructions/instructions.h"
// #else
// #error "You are not compling for aarch64 arhitecture"
// #endif

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
  // TYPE|VALUE|PC|INSTRUCTIONTYPE
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
  UNKNOWN,
  INSTRUCTION,
  DIRECTIVE,
  LABEL_DECLARATION,
} LineType;

void discardComment(char *line);
void trim(char *line);
void preprocessLine(char *line);
LineType getLineType(fields_t *fields);
u8 splitLine(const char *line, fields_t *fields);

// DIRECTIVES
// void global_d(fields_t *fields) {}
// void section_d(fields_t *fields) {}
//
// typedef struct Directive {
//   const Str name;
//   void (*f)(fields_t *fields);
// } Directive;
//
// Directive DIRECTIVES[] = {
//     {{"global", strlen("global"), strlen("global")}, &global_d},
//     {{"section", strlen("section"), strlen("section")}, &global_d},
// };

// INSTRUCTIONS

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
    switch (type = getLineType(&fields)) {
    case DIRECTIVE:
    case INSTRUCTION:
      fprintf(ir_file, "%d|%s|%lu|", type, buf, pc);
      if (type == INSTRUCTION) {
        pc += ARM_INSTRUCTION_SIZE;
        fprintf(ir_file, "%d\n", searchInstruction(&fields));
      } else {
        fprintf(ir_file, "%d\n", NONE);
      }
      break;
    case LABEL_DECLARATION:
      addToSym(fields.fields[0].s, pc, 0, ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE));
      break;
    case UNKNOWN:
      // error here
      break;
    }
  }

  freeFields(&fields);
  fseek(ir_file, 0, SEEK_SET);
  fclose(src);
}

void secondPass(FILE *ir_file, FILE *dst) {
  char buf[LINE_MAX] = {0};
  while (fgets(buf, LINE_MAX, ir_file)) {
    printf("%s", buf);
  }

//  assert(0 && "secondPass is not implemented");
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

void preprocessLine(char *line) {
  discardComment(line);
  trim(line);
}

LineType getLineType(fields_t *fields) {
  char *line = fields->fields[0].s;
  if (line[strlen(line) - 1] == ':') {
    line[strlen(line) - 1] = '\0';
    return LABEL_DECLARATION;
  }

  // add more sophisticated check for directives
  if (*line == '.') {
    return DIRECTIVE;
  }

  InstructionType t = NONE;
  t = searchInstruction(fields);
  if (t == NONE) {
    return UNKNOWN;
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
    case '[':
      if (*i != 0) {
        // if we have char at that string
        // wrap it and go to the next
        fields->fields[fields->n_fields].s[*i] = '\0';
        fields->n_fields++;
        i = &fields->fields[fields->n_fields].len;
      }
      while (*line != ']') {
        fields->fields[fields->n_fields].s[*i] = *line;
        *i += 1;
        line++;
      }
      fields->fields[fields->n_fields].s[*i] = ']';
      *i += 1;
      break;
    default:
      fields->fields[fields->n_fields].s[*i] = *line;
      *i += 1;
      break;
    }
  } while (*line++);

  return fields->n_fields;
}
