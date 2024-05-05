#include "assemble.h"
#include "directives.h"
#include "instructions.h"
#include "lexer.h"
#include "tables.h"
#include "types.h"
#include <elf.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOKEN_MAX (40)

typedef struct Context {
  u64 pc;
  FILE *cur_src;
} Context;

u8 collectLineOfTokens(FILE *src, Fields *f) {
  u8 ok = 1;
  f->n_fields = 0;
  size_t cur_line = LINE;

  while (ok && cur_line == LINE && f->n_fields != FIELDS_MAX + 1) {
    ok = getToken(src, f->fields + f->n_fields);
    if (f->fields[f->n_fields].type == T_LABEL_DECLARATION) {
      f->n_fields--;
      cur_line = LINE;
    }
    f->n_fields++;
  }

  return ok;
}

void makeLabels(Context *c) {
  Token t = initToken(TOKEN_MAX);

  u8 res = 0;
  do {
    res = getToken(c->cur_src, &t);
    switch (t.type) {
    case T_LABEL_DECLARATION:
      if (!addToSym(t.value, c->pc, 0, ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE))) {
        // error here
      }
      break;
    case T_INSTRUCTION:
      c->pc += ARM_INSTRUCTION_SIZE;
      break;
    default:
      (void)NULL;
    }
  } while (res);

  free(t.value);
}

u8 makeAssemble(Context *c, FILE *out) {
  Fields f = initFields(TOKEN_MAX);
  u8 ret = 0;
  fseek(out, ELF64_SIZE, SEEK_SET);

  do {
    ret = collectLineOfTokens(c->cur_src, &f);
    switch (f.fields->type) {
    case T_INSTRUCTION: {
      u32 instruction = assemble(&f);
      if (!instruction) {
        // error here
        break;
      }
      fwrite(&instruction, sizeof(instruction), 1, out);
      c->pc += ARM_INSTRUCTION_SIZE;
      break;
    }
    case T_DIRECTIVE:
      if (!execDirective(&f, c->pc)) {
        // error here
        break;
      }
    default:
      (void)NULL;
    }
  } while (ret);

  freeFields(&f);
  return 1;
}

u8 make(u8 n_sources, const char **sources, const char *out_name) {
  FILE *out = NULL;
  if (!out_name) {
    out_name = alloca(8);
    strcpy((char *)out_name, "a.out");
  }

  out = fopen(out_name, "wb");
  if (!out) {
    fprintf(stderr, "Failed to open %s file. Error: %s\n", out_name,
            strerror(errno));
    return 0;
  }

  initSymbolTable();
  initShdrTable();

  Context c = {0};
  u64 pc = 0;
  for (u8 i = 0; i < n_sources; i++) {
    FILE *src = fopen(sources[i], "rb");
    if (!src) {
      fprintf(stderr, "Failed to open %s file. Error: %s\n", sources[i],
              strerror(errno));
      return 0;
    }
    c.cur_src = src;

    pc = c.pc;
    makeLabels(&c);
    c.pc = pc;
    fseek(src, 0, SEEK_SET);
    makeAssemble(&c, out);
    fclose(src);
  }
  writeElf(out, c.pc);

  freeSymoblTable();
  freeShdrTable();
  fclose(out);
  return 1;
}
