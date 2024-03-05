#include "myasm.h"
#include "parse.h"

void *xmalloc(size_t size);
void firstPass(const char *filename);

int main(int argc, char **argv) {
  if (argc == 1) {
    fprintf(stderr, "Please, provide source file");
    return EXIT_SUCCESS;
  }

  firstPass(argv[1]);
  return EXIT_SUCCESS;
}

void firstPass(const char *filename) {
  FILE *src = fopen(filename, "r");
  if (!src) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  buildTables(src);
  fclose(src);
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
