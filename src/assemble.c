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

u8 makeLabels(FILE *src) {
  Token t = initToken(TOKEN_MAX);
  size_t pc = 0;
  initSymbolTable();

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
      break;
    case T_INSTRUCTION:
      pc += ARM_INSTRUCTION_SIZE;
      break;
    default:
      (void)NULL;
    }
  } while (res);

  free(t.value);
  return ret;
}

u8 collectLineOfTokens(FILE *src, Fields *f) {
  u8 res = 0;
  f->n_fields = 0;
  size_t cur_line = LINE;

  do {
    res = getToken(src, f->fields + f->n_fields);
    if (f->fields[f->n_fields].type == T_LABEL_DECLARATION) {
      f->n_fields--;
      cur_line = LINE;
    }
    f->n_fields++;
  } while (res && cur_line == LINE && f->n_fields != FIELDS_MAX + 1);

  if (!res) {
    f->n_fields--;
  }
  return res;
}

u8 writeData(FILE *ir_file, FILE *out) {
  (void)ir_file;
  (void)out;
  return 1;
}

// first write to ir_file if everything ok
// write to out else
// return 0
u8 makeAssemble(FILE *src, FILE *out) {
  Fields f = initFields(TOKEN_MAX);
  u8 ret = 0;
  size_t pc = 0;
  initShdrTable();
  fseek(out, ELF64_SIZE, SEEK_SET);

  FILE *ir_file = tmpfile();
  do {
    ret = collectLineOfTokens(src, &f);
    switch (f.fields->type) {
    case T_INSTRUCTION: {
      u32 instruction = assemble(&f);
      if (!instruction) {
        // error here
        break;
      }
      fwrite(&instruction, sizeof(instruction), 1, out);
      pc += ARM_INSTRUCTION_SIZE;
      break;
    }
    case T_DIRECTIVE:
      if (!execDirective(&f, pc)) {
        // error here
        break;
      }
    default:
      (void)NULL;
    }
  } while (ret);

  // if (ERRORS.count != 0) {
  // fclose(ir_file);
  // freeFields(&f);
  // return 0;
  // }

  if (!writeElf(out, pc)) {
    // print err
    return 0;
  }
  //  writeData(ir_file, out);

  fclose(ir_file);
  freeFields(&f);
  freeShdrTable();
  freeSymoblTable();
  return 1;
}

u8 make(FILE *src, const char *out_name) {
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

  if (!makeLabels(src)) {
    goto err;
  }
  fseek(src, 0, SEEK_SET);

  if (!makeAssemble(src, out)) {
    goto err;
  }

  return 1;

err:
  fclose(out);
  return 0;
}
