#ifndef MYASM_PARSE
#define MYASM_PARSE

#include "myasm.h"

#define ERROR_SIZE (MAX_TOKEN_SIZE * 2) // token len + 256 for msg
#define ERRORS_LEN (128)

extern u8 N_ERRORS;

typedef struct Error {
  size_t line;
  char msg[ERROR_SIZE];
} Error;

void printError(const Error *err);
__attribute__((nonnull)) void buildTables(FILE *src);

#endif
