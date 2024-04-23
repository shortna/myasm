#include "assemble.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  if (argc == 1) {
    fprintf(stderr, "Please, provide source file");
    return EXIT_SUCCESS;
  }

  FILE *src = fopen(argv[1], "rb");
  if (!src) {
    fprintf(stderr, "Failed to open ir file. Error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  firstPass(src);
  fclose(src);

  secondPass();

  return EXIT_SUCCESS;
}

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
