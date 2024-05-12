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

  u8 section_ind = 0;
  u64 pc = 0;
  while (getToken(CONTEXT.cur_src, &t)) {
    switch (t.type) {
    case T_LABEL_DECLARATION:
      if (!addToSym(t.value, pc, 0, ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE), section_ind)) {
        // error here
      }
      break;
    case T_INSTRUCTION:
      pc += ARM_INSTRUCTION_SIZE;
      break;
    case T_DIRECTIVE: {
      DirectiveType d = searchDirective(t.value + 1);
      switch (d) {
      case D_SECTION:
        section_ind++;
        __attribute__((fallthrough));
      case D_NONE:
      case D_GLOBAL:
        break;
      case D_BYTE:
        pc += sizeof(char);
        break;
      case D_INT:
        pc += sizeof(int);
        break;
      case D_ASCIIZ:
        pc += 1;
        __attribute__((fallthrough));
      case D_ASCII:
      case D_ZERO:
        getToken(CONTEXT.cur_src, &t);
        pc += getDirectiveSize(t.value);
        break;
      }
      break;
    default:
      NULL;
    }
    }
  }

  CONTEXT.pc = 0;
  free(t.value);
}

u8 makeAssemble(void) {
  Fields f = initFields(TOKEN_SIZE);
  u8 ret = 0;
  fseek(CONTEXT.out, ELF64_HEADER_SIZE, SEEK_SET);

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
      switch (searchDirective(f.fields->value + 1)) {
      case D_SECTION:
        CONTEXT.cur_sndx++;
        __attribute__((fallthrough));
      default:
        if (!execDirective(&f)) {
          // error here
        }
        break;
      }
    default:
      NULL;
    }
  } while (ret);

  freeFields(&f);
  return 1;
}

u8 make(const char *sources, const char *out_name) {
  CONTEXT.out = NULL;
  if (!out_name) {
    out_name = alloca(8);
    strcpy((char *)out_name, "a.out");
  }

  CONTEXT.out = fopen(out_name, "wb");
  if (!CONTEXT.out) {
    fprintf(stderr, "Failed to open %s file. Error: %s\n", out_name,
            strerror(errno));
    return 0;
  }

  CONTEXT.cur_src = fopen(sources, "rb");
  if (!CONTEXT.cur_src) {
    fclose(CONTEXT.out);
    fprintf(stderr, "Failed to open %s file. Error: %s\n", out_name,
            strerror(errno));
    return 0;
  }

  initSymbolTable();
  initShdrTable();
  makeLabels();

  fseek(CONTEXT.cur_src, 0, SEEK_SET);
  initRelocationTable();
  makeAssemble();

  if (!writeElf(CONTEXT.out)) {
    // error here
  }
  fclose(CONTEXT.cur_src);
  fclose(CONTEXT.out);

  freeSymoblTable();
  freeShdrTable();
  freeRelocationTable();
  return 1;
}
