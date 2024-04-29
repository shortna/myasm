#include "directives.h"
#include "instructions_api.h"
#include "tables.h"
#include "types.h"
#include <elf.h>
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

Flags FLAGS[] = {
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

u8 dSection(const Fields *f, size_t offset) {
  if (f->n_fields != 3) {
    // error here
    return 0;
  }

  if (searchInShdr(f->fields[1].value) != -1) {
    // error here
    return 0;
  }

  u64 flags;
  if (!decodeFlags(f->fields[2].value, &flags)) {
    return 0;
  }

  addToShdr(f->fields[1].value, SHT_PROGBITS, flags, offset, 0, 0, 0);
  return 1;
}

const char *DIRECTIVES[] = {"global", "section"};

i8 searchDirective(const char *name) {
  for (size_t i = 0; i < sizeof(DIRECTIVES) / sizeof(*DIRECTIVES); i++) {
    if (strcmp(DIRECTIVES[i], name) == 0) {
      return i;
    }
  }
  return -1;
}

u8 execDirective(Fields *f, size_t pc) {
  i8 n = searchDirective(f->fields->value + 1);
  u8 res = 0;

  switch (n) {
  case -1:
    break;
  case 0:
    res = dGlobal(f);
    break;
  case 1:
    res = dSection(f, pc);
    break;
  }

  return res;
}
