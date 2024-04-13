#include "myasm.h"
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

  fseek(ir_file, 0, SEEK_SET);
  fclose(src);
}

void secondPass(FILE *ir_file, FILE *dst) {
  // check if errors
  // if (ERRORS.len > 0) {fprint("err"), return}
}
