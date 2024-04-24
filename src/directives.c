#include "directives.h"
#include "instructions_api.h"
#include "tables.h"
#include "types.h"
#include <ctype.h>
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

u8 dSection(const Fields *f, size_t offset) {
  if (f->n_fields != 3) {
    // error here
    return 0;
  }

  if (searchInShdr(f->fields[1].value) != -1) {
    // error here
    return 0;
  }

  u8 flags = 0;
  const char *str_flags = f->fields[2].value + 1;
  while (*str_flags && *str_flags != '"') {
    if (!isupper(*str_flags)) {
      return 0;
    }

    flags |= *str_flags - 'A';
    str_flags++;
  }
  if (flags >= 8) {
    // error here
    return 0;
  }

  u8 allign = flags & SHF_EXECINSTR ? ARM_INSTRUCTION_SIZE : 1; // if executable allign to 4
  addToShdr(f->fields[1].value, SHT_PROGBITS, flags, offset, 0, 0, allign, 0);
  addToSym(f->fields[1].value, 0, 0, STT_SECTION);
  return 1;
}

const char *DIRECTIVES[] = {"GLOBAL", "SECTION"};

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
