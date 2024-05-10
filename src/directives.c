#include "directives.h"
#include "instructions.h"
#include "parser.h"
#include "tables.h"
#include "types.h"
#include <elf.h>
#include <stdio.h>
#include <string.h>

u8 dGlobal(const Fields *f) {
  if (f->n_fields != 2) {
    // error here
    return 0;
  }

  ssize_t n = searchInSym(f->fields[1].value);
  if (n == -1) {
    // error here
    return 0;
  }

  changeBinding(n, STB_GLOBAL);
  return 1;
}

//  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
//  L (link order), O (extra OS processing required), G (group), T (TLS),
//  C (compressed), x (unknown), o (OS specific), E (exclude),
//  D (mbind), p (processor specific)
typedef struct Flags {
  const char name;
  const u16 value;
} Flags;

static const Flags FLAGS[] = {
    {'w', SHF_WRITE},      {'a', SHF_ALLOC},
    {'x', SHF_EXECINSTR},  {'m', SHF_MERGE},
    {'s', SHF_STRINGS},    {'i', SHF_INFO_LINK},
    {'l', SHF_LINK_ORDER}, {'o', SHF_OS_NONCONFORMING},
    {'g', SHF_GROUP},      {'t', SHF_TLS},
    {'c', SHF_COMPRESSED},
};

u8 decodeFlags(const char *flags, u64 *res) {
  *res = 0;
  flags++;
  while (*flags && *flags != '"') {
    bool valid_flag = false;
    for (u8 i = 0; i < sizeof(FLAGS) / sizeof(*FLAGS); i++) {
      if (*flags == FLAGS[i].name) {
        valid_flag = true;
        *res |= FLAGS[i].value;
      }
    }
    if (!valid_flag) {
      return 0;
    }
    flags++;
  }
  return 1;
}

u8 dSection(const Fields *f) {
  if (f->n_fields != 3) {
    // error here
    return 0;
  }

  u64 flags;
  if (!decodeFlags(f->fields[2].value, &flags)) {
    return 0;
  }

  addToShdr(f->fields[1].value, SHT_PROGBITS, flags, CONTEXT.pc, 0, 0, 0);
  return 1;
}

u8 dZero(const Fields *f) {
  u64 n;
  if (!parseImmediateU64(f->fields[1].value, &n)) {
    return 0;
  }

  fwrite(0, sizeof(0), n, CONTEXT.out);
  CONTEXT.pc += n * sizeof(0);
  return 1;
}

u8 dByte(const Fields *f) { 
  u8 n;
  if (!parseImmediateU8(f->fields[1].value, &n)) {
    return 0;
  }

  fwrite(&n, sizeof(n), 1, CONTEXT.out);
  CONTEXT.pc += sizeof(n);
  return 1;
}

u8 dInt(const Fields *f) {
  u32 n;
  if (!parseImmediateU32(f->fields[1].value, &n)) {
    return 0;
  }

  fwrite(&n, sizeof(n), 1, CONTEXT.out);
  CONTEXT.pc += sizeof(n);
  return 1;
}

u8 dAscii(const Fields *f) { 
  u64 len = 0;
  const char *str = f->fields[1].value + 1; // skips first '"'
  while (*str != '"') {
    len++;
  }

  fwrite(str, len, 1, CONTEXT.out);
  CONTEXT.pc += len * sizeof(char);
  return 1;
}

u8 dAsciiz(const Fields *f) { 
  if (!dAscii(f)) {
    return 0;
  }

  fwrite(0, sizeof(char), 1, CONTEXT.out); // write '\0'
  CONTEXT.pc += sizeof(char);
  return 1;
}

static const char *DIRECTIVES[] = {
    "global", "section", "byte", "int", "ascii", "asciiz", "zero",
};

i8 searchDirective(const char *name) {
  for (size_t i = 0; i < sizeof(DIRECTIVES) / sizeof(*DIRECTIVES); i++) {
    if (strcmp(DIRECTIVES[i], name) == 0) {
      return i;
    }
  }
  return -1;
}

u8 execDirective(Fields *f) {
  i8 n = searchDirective(f->fields->value + 1);
  u8 res = 0;

  switch (n) {
  case -1:
    // error here
    break;
  case 0:
    res = dGlobal(f);
    break;
  case 1:
    res = dSection(f);
    break;
  case 2:
    res = dByte(f);
    break;
  case 3:
    res = dInt(f);
    break;
  case 4:
    res = dAscii(f);
    break;
  case 5:
    res = dAsciiz(f);
    break;
  case 6:
    res = dZero(f);
    break;
  }

  return res;
}
