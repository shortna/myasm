#include "directives.h"
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

u8 getDirectiveSize(const char *directive_arg) {
  if (*directive_arg == '"') {
    u64 len = 0;
    directive_arg += 1;
    while (directive_arg[len] != '"') {
      len++;
    }
    return len;
  }

  u64 n = 0;
  if (!parseImmediateU64(directive_arg, &n)) {
    return 0;
  }
  return n * sizeof(0);
}

u8 dZero(const Fields *f) {
  u64 n;
  if (!parseImmediateU64(f->fields[1].value, &n)) {
    return 0;
  }

  int z = 0;
  for (u64 i = 0; i < n; i++) {
    fwrite(&z, sizeof(z), 1, CONTEXT.out);
  }
  CONTEXT.pc += n * sizeof(z);
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
  while (str[len] != '"') {
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

  char z = 0;
  fwrite(&z, sizeof(z), 1, CONTEXT.out); // write '\0'
  CONTEXT.pc += sizeof(z);
  return 1;
}

DirectiveType searchDirective(const char *name) {
  for (size_t i = 0; i < sizeof(DIRECTIVES) / sizeof(*DIRECTIVES); i++) {
    if (strcmp(DIRECTIVES[i].name, name) == 0) {
      return DIRECTIVES[i].t;
    }
  }
  return D_NONE;
}

u8 execDirective(Fields *f) {
  DirectiveType t = searchDirective(f->fields->value + 1);

  switch (t) {
  case D_NONE:
    return 0;
  case D_GLOBAL:
    return dGlobal(f);
  case D_SECTION:
    return dSection(f);
  case D_BYTE:
    return dByte(f);
  case D_INT:
    return dInt(f);
  case D_ASCII:
    return dAscii(f);
  case D_ASCIIZ:
    return dAsciiz(f);
    break;
  case D_ZERO:
    return dZero(f);
  }
}
