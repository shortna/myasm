#include "myasm.h"
#include "instructions/lexer.h"
#include "tables/tables.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  firstPass(argv[1], ir_file);
  secondPass(ir_file, NULL);

  fclose(ir_file);
  return EXIT_SUCCESS;
}

void firstPass(const char *filename, FILE *ir_file) {
  FILE *src = fopen(filename, "r");
  if (!src) {
    fprintf(stderr, "Failed to open src file. Error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  initTables();
  size_t pc = 0;

  Token t = initToken(UINT8_MAX);
  Fields f = initFields(UINT8_MAX);

  u8 err = 0;
  err = getToken(src, &t);
  do {
    switch (t.type) {
    case T_LABEL_DECLARATION:
      addToSym(t.value, pc, 0, ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE));
      getToken(NULL, &t);
      break;

    case T_INSTRUCTION:
      pc += 4;
      __attribute__((fallthrough));
    case T_DIRECTIVE:
      do {
        if (f.n_fields < FIELDS_MAX) {
          copyToken(f.fields + f.n_fields, &t);
          f.n_fields++;
        } else {
          break;
        }
        err = getToken(NULL, &t);
      } while (err && t.type != T_DIRECTIVE && t.type != T_INSTRUCTION &&
               t.type != T_LABEL_DECLARATION);

      fwrite(&f.n_fields, sizeof(f.n_fields), 1, ir_file);
      for (u8 i = 0; i < f.n_fields; i++) {
        writeToken(ir_file, f.fields + i);
      }

      f.n_fields = 0;
      break;
    }
  } while (err);
  fclose(src);

  free(t.value);
  freeFields(&f);
  fseek(ir_file, 0, SEEK_SET);
}

void secondPass(FILE *ir_file, FILE *dst) {
  u8 n_tokens = 0;
  Token t = initToken(UINT8_MAX);

  while (1) {
    fread(&n_tokens, sizeof(n_tokens), 1, ir_file);
    for (u8 i = 0; i < n_tokens; i++) {
      if (!readToken(ir_file, &t)) {
        goto done;
      }
      printf("%4d %4lu %20s\n", t.type, t.capacity, t.value);
    }
  }

done:
  free(t.value);
}
