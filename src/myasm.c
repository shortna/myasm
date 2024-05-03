#include "assemble.h"
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const struct option LONG_OPTIONS[] = {
    {"o", required_argument, NULL, 'o'},
    {0, 0, 0, 0},
};

int main(int argc, char **argv) {
  if (argc == 1) {
    fprintf(stderr, "Please, provide source file");
    return EXIT_SUCCESS;
  }

  FILE *src = fopen(argv[1], "rb");
  if (!src) {
    fprintf(stderr, "Failed to open \"%s\" file. Error: %s\n", argv[1],
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  int opt = 0;
  const char *out_name = NULL;
  while ((opt = getopt_long(argc, argv, "o", LONG_OPTIONS, NULL)) != -1) {
    switch (opt) {
    case 'o':
      out_name = optarg;
      break;
    default:
      (void)NULL;
    }
  }

  make(src, out_name);
  fclose(src);
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

void *xcalloc(size_t nmemb, size_t size) {
  void *p = calloc(nmemb, size);
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
