#include "parse.h"
#include "build_binary.h"
#include "lexer.h"

#ifdef DEBUG
#define SRC_FILE "parse.c"
#endif

u8 N_ERRORS = 0;
Error *errors = NULL;

static size_t LC = 0;
static FILE *ir_file = NULL;

void buildTables(FILE *src) {
  ir_file = tmpfile();
  if (!ir_file) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  initTables();

  getToken(src, NULL); // sets default file
  Token token = {0};

  while (getToken(NULL, &token) != EOF) {
    switch (token.type) {
    case LABEL_DECLARATION:
    case INSTRUCTION_OR_LABEL:
    case DIRECTIVE:
    default:
      break;
    }
  }

  fseek(ir_file, 0, SEEK_SET);
}
