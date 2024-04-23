#include "assemble.h"
#include "directives.h"
#include "instructions_api.h"
#include "lexer.h"
#include "tables.h"
#include "types.h"
#include <elf.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOKEN_MAX (40)

u8 makeLabels(FILE *src) {
  Token t = initToken(TOKEN_MAX);
  size_t pc = 0;

  u8 ret = 1;
  u8 res = 0;
  do {
    res = getToken(src, &t);
    switch (t.type) {
    case T_LABEL_DECLARATION:
      if (!addToSym(t.value, pc, 0, ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE))) {
        // error here
        ret = 0;
      }
    case T_INSTRUCTION:
      pc += ARM_INSTRUCTION_SIZE;
    default:
      (void)NULL;
    }
  } while (res);

  free(t.value);
  return ret;
}

u8 collectLineOfTokens(FILE *src, Fields *f) {
  u8 res = 1;
  TokenType t = T_NONE;
  f->n_fields = 0;

  return res;
}

u8 makeAssemble(FILE *src, FILE *out) {
  Fields f = initFields(TOKEN_MAX);
  u8 ret = 0;

  do {
    ret = collectLineOfTokens(src, &f);
    for (u8 i = 0; i < f.n_fields; i++) {
      printf("%s ", f.fields[i].value);
    }
    printf("\n");
  } while (ret);

  freeFields(&f);
  return 1;
}

u8 make(FILE *src, char *out_name) {
  //  FILE *out = NULL;
  //  if (!out_name) {
  //    out_name = alloca(10);
  //    strcpy(out_name, "a.out");
  //    out = fopen("a.out", "wb");
  //  } else {
  //    out = fopen(out_name, "wb");
  //  }
  //
  //  if (!out) {
  //    fprintf(stderr, "Failed to open %s file. Error: %s\n", out_name,
  //            strerror(errno));
  //    return 0;
  //  }

  u8 ret = 1;
  ret = makeLabels(src);
  fseek(src, 0, SEEK_SET);

  //  ret = ret ? makeAssemble(src, out) : 0;
  ret = ret ? makeAssemble(src, NULL) : 0;
  //  fclose(out);

  return ret;
}
