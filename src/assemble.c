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

#ifndef alloca
#define alloca(size) __builtin_alloca(size)
#endif

u8 collectLineOfTokens(Fields *f) {
  static Token LEFTOVER = {0};

  u8 ok;
  if (LEFTOVER.value) {
    f->n_fields = 1;
    copyToken(f->fields, &LEFTOVER);
  } else {
    f->n_fields = 0;
  }

  do {
    ok = getToken(CONTEXT.cur_src, f->fields + f->n_fields);
    if (!ok) {
      break;
    }

    switch (f->fields[f->n_fields].type) {
    case T_LABEL_DECLARATION:
      break;
    case T_INSTRUCTION:
    case T_DIRECTIVE:
      if (f->n_fields != 0) {
        copyToken(&LEFTOVER, f->fields + f->n_fields);
        goto done;
      }
      __attribute__((fallthrough));
    default:
      f->n_fields++;
    }
  } while (ok && f->n_fields != FIELDS_MAX);

done:
  if (!ok && LEFTOVER.value) {
    free(LEFTOVER.value);
    LEFTOVER.value = NULL;
  }

  return ok;
}

void makeLabels(void) {
  Token t = initToken(TOKEN_SIZE);

  size_t pc = 0;
  while (getToken(CONTEXT.cur_src, &t)) {
    switch (t.type) {
    case T_LABEL_DECLARATION:
      if (!addToSym(t.value, pc, 0, ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE))) {
        // error here
      }
      break;
    case T_INSTRUCTION:
      pc += ARM_INSTRUCTION_SIZE;
      break;
    case T_DIRECTIVE:
      pc += 0; // fix this
    default:
      NULL;
    }
  }

  free(t.value);
}

u8 makeAssemble(void) {
  Fields f = initFields(TOKEN_SIZE);
  u8 ret = 0;
  fseek(CONTEXT.out, ELF64_SIZE, SEEK_SET);

  do {
    ret = collectLineOfTokens(&f);
    switch (f.fields->type) {
    case T_INSTRUCTION: {
      u32 instruction = assemble(&f);
      if (!instruction) {
        // error here
        break;
      }
      CONTEXT.pc += ARM_INSTRUCTION_SIZE;
      fwrite(&instruction, sizeof(instruction), 1, CONTEXT.out);
      break;
    }
    case T_DIRECTIVE:
      if (!execDirective(&f)) {
        // error here
        break;
      }
    default:
      NULL;
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
  CONTEXT.out = out;

  initSymbolTable();
  initShdrTable();

  for (u8 i = 0; i < n_sources; i++) {
    FILE *src = fopen(sources[i], "rb");
    if (!src) {
      fprintf(stderr, "Failed to open %s file. Error: %s\n", sources[i],
              strerror(errno));
      return 0;
    }
    CONTEXT.cur_src = src;

    makeLabels();
    fseek(CONTEXT.cur_src, 0, SEEK_SET);
    makeAssemble();
    fclose(CONTEXT.cur_src);
  }
  writeElf(CONTEXT.out);

  freeSymoblTable();
  freeShdrTable();
  fclose(CONTEXT.out);
  return 1;
}
