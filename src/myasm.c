#include "myasm.h"
#include "assemble.h"
#include "types.h"
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

bool ERRORS = false;

const char *source = NULL;
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

  for (int i = 1; i < argc; i++) {
    if (access(argv[i], F_OK) == 0) {
      if (!out_name || strcmp(out_name, argv[i]) != 0) {
        source = argv[i];
      }
    }
  }

  if (source == NULL) {
    fprintf(stderr, "Please, provide VALID source file\n");
    return EXIT_FAILURE;
  }

  if (!make(source, out_name)) {
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

void printError(const char *msg, const Fields *f) {
  ERRORS = true;

  fprintf(stderr, "%s: %s\n", source, msg);
  fprintf(stderr, "%s:%lu:%s ", source, LINE, f->fields->value);
  for (u8 i = 1; i < f->n_fields - 1; i++) {
    fprintf(stderr, "%s,", f->fields[i].value);
  }
  fprintf(stderr, "%s\n", f->fields[f->n_fields - 1].value);
}
