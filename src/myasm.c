#include "common/myasm.h"
#include "parser/parser.h"

void *xmalloc(size_t size);
void run(const char *filename);

int main(int argc, char **argv) {
  if (argc == 1) {
    fprintf(stderr, "Please, provide source file");
    return EXIT_SUCCESS;
  }

  run(argv[1]);
  return EXIT_SUCCESS;
}

void run(const char *filename) {
  FILE *src = fopen(filename, "r");
  if (!src) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  firstPass(src);
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
