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

u8 collectLineOfTokens(Fields *f) {
  f->n_fields = 0;
  u8 ok = getToken(CONTEXT.cur_src, f->fields);
  while (ok &&
         (f->fields->type == T_EOL || f->fields->type == T_LABEL_DECLARATION)) {
    ok = getToken(CONTEXT.cur_src, f->fields);
  }

  if (!ok) {
    return ok;
  }

  while (ok && f->n_fields != FIELDS_SIZE - 1 &&
         f->fields[f->n_fields].type != T_EOL) {
    f->n_fields++;
    ok = getToken(CONTEXT.cur_src, f->fields + f->n_fields);
  }
  f->n_fields++;

  return ok;
}

u8 makeLabels(void) {
  Token t = initToken(TOKEN_SIZE);

  u8 section_ind = 0;
  u64 pc = 0;
  while (getToken(CONTEXT.cur_src, &t)) {
    switch (t.type) {
    case T_LABEL_DECLARATION:
      if (!addToSym(t.value, pc, 0, ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE),
                    section_ind)) {
        errorToken("Label already defined", &t);
      }
      break;
    case T_INSTRUCTION:
      pc += ARM_INSTRUCTION_SIZE;
      break;
    case T_DIRECTIVE:
      switch (searchDirective(t.value + 1)) {
      case D_SECTION:
        section_ind++;
        pc = 0;
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
      default:
        NULL;
      }
    default:
      NULL;
    }
  }

  CONTEXT.pc = 0;
  LINE = 0;
  free(t.value);
  if (ERRORS) {
    return 0;
  } 
  return 1;
}

u8 makeAssemble(void) {
  Fields f = initFields(TOKEN_SIZE);
  fseek(CONTEXT.out, ELF64_HEADER_SIZE, SEEK_SET);

  u8 ret = collectLineOfTokens(&f);
  while (ret) {
    switch (f.fields->type) {
    case T_INSTRUCTION: {
      u32 instruction = assemble(&f);
      if (!instruction) {
        break;
      }
      CONTEXT.pc += ARM_INSTRUCTION_SIZE;
      fwrite(&instruction, sizeof(instruction), 1, CONTEXT.out);
      break;
    }
    case T_DIRECTIVE:
      if (searchDirective(f.fields->value + 1) == D_SECTION) {
        CONTEXT.cur_sndx++;
        CONTEXT.size += CONTEXT.pc;
        CONTEXT.pc = 0;
      }
      execDirective(&f);
      break;
    case T_EOL:
      break;
    case T_LABEL:
    case T_NONE:
    case T_LABEL_DECLARATION:
    case T_REGISTER:
    case T_IMMEDIATE:
    case T_SHIFT:
    case T_EXTEND:
    case T_CONDITION:
    case T_STRING:
    case T_RSBRACE:
    case T_LSBRACE:
    case T_BANG:
    case T_EQUAL:
      if (*f.fields->value == '.') {
        errorFields("Unknown directive", &f);
      } else {
        errorFields("Unknown instruction", &f);
      }
      break;
    }
    ret = collectLineOfTokens(&f);
  }

  CONTEXT.size += CONTEXT.pc;
  freeFields(&f);
  return 1;
}

u8 make(const char *sources, const char *out_name) {
  CONTEXT.out = NULL;
  if (!out_name) {
    out_name = alloca(8);
    strcpy((char *)out_name, "a.out");
  }
  u8 res = 1;

  CONTEXT.out = tmpfile();
  if (!CONTEXT.out) {
    fprintf(stderr, "Failed to open internal file. Error: %s\n",
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

  initShdrTable();
  initSymbolTable();
  initRelocationTable();

  if (!makeLabels()) {
    res = 0;
    goto done;
  }

  fseek(CONTEXT.cur_src, 0, SEEK_SET);
  makeAssemble();

  writeElf(CONTEXT.out);

  if (!ERRORS) {
    FILE *out = fopen(out_name, "w");
    if (!out) {
      fclose(CONTEXT.out);
      fprintf(stderr, "Failed to open %s file. Error: %s\n", out_name,
              strerror(errno));
      res = 0;
      goto done;
    } else {
      fseek(CONTEXT.out, 0, SEEK_SET);
      fseek(out, 0, SEEK_SET);
      int ch;
      while ((ch = fgetc(CONTEXT.out)) != EOF) {
        fputc(ch, out);
      }
    }
    fclose(out);
  }

done:
  fclose(CONTEXT.cur_src);
  fclose(CONTEXT.out);
  freeSymoblTable();
  freeShdrTable();
  freeRelocationTable();
  return res;
}
