#include "myasm.h"
#include "assemble.h"
#include "types.h"
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

void error(const char *msg) {
  ERRORS = true;
  fprintf(stderr, "%s:%lu: ERROR: %s -- ", source, LINE, msg);
}

void errorToken(const char *msg, const Token *t) {
  error(msg);
  fprintf(stderr, "%s\n", t->value);
}

void errorFields(const char *msg, const Fields *f) {
  error(msg);
  u8 len = f->fields[f->n_fields - 1].type == T_EOL ? f->n_fields - 1 : f->n_fields;
  for (u8 i = 0; i < len; i++) {
    fprintf(stderr, "%s ", f->fields[i].value);
  }
  fprintf(stderr, "\n");
}
