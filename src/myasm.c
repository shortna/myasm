#include "assemble.h"
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_NUMBER_FILES (20)

static const struct option LONG_OPTIONS[] = {
    {"o", required_argument, NULL, 'o'},
    {0, 0, 0, 0},
};

int main(int argc, char **argv) {
  if (argc == 1) {
    fprintf(stderr, "Please, provide source file");
    return EXIT_FAILURE;
  }

  int opt = 0;
  const char *out_name = NULL;
  while ((opt = getopt_long(argc, argv, "o:", LONG_OPTIONS, NULL)) != -1) {
    switch (opt) {
    case 'o':
      out_name = optarg;
      break;
    case '?':
      return EXIT_FAILURE;
    default:
      NULL;
    }
  }

  const char *sources[MAX_NUMBER_FILES];
  u8 n_sources = 0;
  for (int i = 1; i < argc; i++) {
    if (access(argv[i], F_OK) == 0) {
      if (!out_name || strcmp(out_name, argv[i]) != 0) {
        sources[n_sources] = argv[i];
        n_sources++;
      }
    }
  }

  if (n_sources == 0) {
    fprintf(stderr, "Please, provide source file");
    return EXIT_FAILURE;
  }

  if (!make(n_sources, sources, out_name)) {
    return EXIT_FAILURE;
  }

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
