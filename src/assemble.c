#include "assemble.h"
#include "elf.h"
#include "lexer.h"
#include "tables.h"
#include "types.h"
#include "instructions_api.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printBinary(u32 n) {
  for (u32 i = 0; i < sizeof(n) * 8; i++) {
    printf("%d", (n >> ((sizeof(n) * 8) - i - 1)) & 1);
  }
  printf("\n");
}

// ir_file
// TYPE|VALUE|PC
FILE *ir_file = NULL;

u8 firstPass(FILE *src) {
  if (!src) {
    return 0;
  }

  ir_file = tmpfile();
  if (!ir_file) {
    return 0;
  }

  initTables();
  size_t pc = 0;

  Token t = initToken(UINT8_MAX);
  Fields f = initFields(UINT8_MAX);

  u8 err = 0;
  err = getToken(src, &t);
  do {
    switch (t.type) {
    case T_LABEL_DECLARATION:
      addToSym(t.value, pc, 0, ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE));
      getToken(NULL, &t);
      break;

    case T_INSTRUCTION:
      pc += 4;
      __attribute__((fallthrough));
    case T_DIRECTIVE:
      do {
        if (f.n_fields == FIELDS_MAX) {
          break;
        } 
        copyToken(f.fields + f.n_fields, &t);
        f.n_fields++;

        err = getToken(NULL, &t);
      } while (err && t.type != T_DIRECTIVE && t.type != T_INSTRUCTION &&
               t.type != T_LABEL_DECLARATION);

      fwrite(&f.n_fields, sizeof(f.n_fields), 1, ir_file);
      for (u8 i = 0; i < f.n_fields; i++) {
        writeToken(ir_file, f.fields + i);
      }

      f.n_fields = 0;
      break;
    }
  } while (err);

  free(t.value);
  freeFields(&f);
  fseek(ir_file, 0, SEEK_SET);
  return 1;
}

u8 secondPass(void) {
  Fields f = initFields(UINT8_MAX);
  //  fseek(dst, ELF64_SIZE, SEEK_SET);

  u8 err = 0;
  do {
    fread(&f.n_fields, sizeof(f.n_fields), 1, ir_file);
    for (u8 i = 0; i < f.n_fields; i++) {
      err = readToken(ir_file, f.fields + i);
    }
    if (!err) {
      break;
    }

    if (f.fields->type == T_INSTRUCTION) {
      u32 instruction = assemble(&f);
      if (instruction != 0) {
        printf("%u\n", instruction);
      }
    }
  } while (err);

  freeFields(&f);
  fclose(ir_file);
  
  return 1;
}
