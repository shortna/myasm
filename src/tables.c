#include "tables.h"
#include "instructions.h"
#include "types.h"
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TALBE_START_SIZE (10)

typedef struct SymTable {
  char *strtab;
  Elf64_Sym *items;
  size_t count;
  size_t capacity;
} SymTable;

typedef struct ShdrTable {
  char *shstrtab;
  Elf64_Shdr *items;
  size_t count;
  size_t capacity;
} ShdrTable;

static SymTable SYMBOLS = {0};
static ShdrTable SECTIONS = {0};

void initSymbolTable(void) {
  SYMBOLS.items = xmalloc(TALBE_START_SIZE * sizeof(*SYMBOLS.items));
  SYMBOLS.strtab =
      xcalloc(TALBE_START_SIZE, TOKEN_SIZE * sizeof(*SYMBOLS.strtab));

  SYMBOLS.count = 0;
  SYMBOLS.capacity = TALBE_START_SIZE;
  addToSym("", 0, 0, 0);
  SYMBOLS.items[0].st_name = 0;
}

void initShdrTable(void) {
  SECTIONS.items = xmalloc(TALBE_START_SIZE * sizeof(*SECTIONS.items));
  SECTIONS.shstrtab =
      xcalloc(TALBE_START_SIZE, TOKEN_SIZE * sizeof(*SECTIONS.shstrtab));

  SECTIONS.count = 0;
  SECTIONS.capacity = TALBE_START_SIZE;
  addToShdr("", SHT_NULL, 0, 0, 0, 0, 0);
  SECTIONS.items[0].sh_name = 0;
}

void freeSymoblTable(void) {
  free(SYMBOLS.items);
  free(SYMBOLS.strtab);
}

void freeShdrTable(void) {
  free(SECTIONS.items);
  free(SECTIONS.shstrtab);
}

ssize_t searchTable(const char *needle, const char *strtable, size_t count) {
  strtable++; // skip first entry cause its always \0
  size_t i = 1;
  while (i < count) {
    if (strcmp(needle, strtable) == 0) {
      return i;
    }
    strtable += strlen(strtable) + 1;
    i++;
  }
  return -1;
}

ssize_t searchInSym(const char *needle) {
  if (!needle) {
    return -1;
  }

  ssize_t res = searchTable(needle, SYMBOLS.strtab, SYMBOLS.count);
  return res;
}

ssize_t searchInShdr(const char *needle) {
  if (!needle) {
    return -1;
  }

  ssize_t res = searchTable(needle, SECTIONS.shstrtab, SECTIONS.count);
  return res;
}

void resizeSymTable(void) {
  SYMBOLS.capacity *= 2;
  SYMBOLS.items =
      xrealloc(SYMBOLS.items, SYMBOLS.capacity * sizeof(*SYMBOLS.items));

  SYMBOLS.strtab = xrealloc(SYMBOLS.strtab, SYMBOLS.capacity * TOKEN_SIZE *
                                                sizeof(*SYMBOLS.strtab));
}

void resizeShdrTable(void) {
  SECTIONS.capacity *= 2;
  SECTIONS.items =
      xrealloc(SECTIONS.items, SECTIONS.capacity * sizeof(*SECTIONS.items));

  SECTIONS.shstrtab =
      xrealloc(SECTIONS.shstrtab,
               SECTIONS.capacity * TOKEN_SIZE * sizeof(*SECTIONS.shstrtab));
}

ssize_t getLabelPc(const char *needle) {
  ssize_t res = searchInSym(needle);
  if (res == -1) {
    return res;
  }
  return SYMBOLS.items[res].st_value; // return pc
}

// concats s to table with delemiter between '\0' them
u8 concatStrs(char *table, const char *s, size_t count) {
  table++;
  size_t i = 1;
  while (i < count) {
    table += strlen(table) + 1;
    i++;
  }

  strncat(table, s, TOKEN_SIZE - 1);
  return 1;
}

size_t getStringTableSize(const char *table);

u8 addToSym(const char *name, Elf64_Addr st_value, u64 st_size, u8 st_info) {
  ssize_t res = searchInSym(name);
  if (res != -1) {
    return 0;
  }

  if (SYMBOLS.count == SYMBOLS.capacity + 1) {
    resizeSymTable();
  }
  Elf64_Sym *item = SYMBOLS.items + SYMBOLS.count;
  item->st_name = getStringTableSize(SYMBOLS.strtab);
  item->st_value = st_value;
  item->st_size = st_size;
  item->st_info = ELF64_ST_INFO(STB_LOCAL, st_info);
  item->st_other = STV_DEFAULT;
  item->st_shndx = 0;
  concatStrs(SYMBOLS.strtab, name, SYMBOLS.count);
  SYMBOLS.count++;
  return 1;
}

u8 addToShdr(const char *name, u32 sh_type, u64 sh_flags, Elf64_Off sh_offset,
             u32 sh_link, u32 sh_info, u64 sh_entsize) {

  ssize_t res = searchInShdr(name);
  if (res != -1) {
    return 0;
  }

  if (SECTIONS.count == SECTIONS.capacity) {
    resizeShdrTable();
  }
  Elf64_Shdr *item = SECTIONS.items + SECTIONS.count;
  item->sh_name = getStringTableSize(SECTIONS.shstrtab);
  item->sh_type = sh_type;
  item->sh_flags = sh_flags;
  item->sh_addr = 0;
  item->sh_offset = sh_offset;
  item->sh_size = 0;
  item->sh_link = sh_link;
  item->sh_info = sh_info;
  item->sh_addralign = 0;
  item->sh_entsize = sh_entsize;
  concatStrs(SECTIONS.shstrtab, name, SECTIONS.count);
  SECTIONS.count++;
  return 1;
}

u8 changeBinding(size_t ind, u8 binding) {
  if (ind >= SYMBOLS.count) {
    return 0;
  }

  u8 current_type = SYMBOLS.items[ind].st_info & 0xf;
  SYMBOLS.items[ind].st_info = ELF64_ST_INFO(binding, current_type);
  return 1;
}

size_t getStringTableSize(const char *table) {
  const char *table_s = table;
  table++;
  while (*table) {
    table += strlen(table) + 1;
  }
  return table - table_s;
}

size_t getSectionsEnd(void) {
  return SECTIONS.items[SECTIONS.count - 1].sh_offset +
         SECTIONS.items[SECTIONS.count - 1].sh_size;
}

u8 writeHeader(FILE *out) {
  Elf64_Off e_shoff = getSectionsEnd();
  Elf64_Ehdr h = {{ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3, ELFCLASS64, ELFDATA2LSB,
                   EV_CURRENT, ELFOSABI_SYSV, 0, 0, 0, 0, 0, 0, 0, EI_NIDENT},
                  ET_REL,
                  EM_AARCH64,
                  EV_CURRENT,
                  0,
                  0,
                  e_shoff,
                  0,
                  ELF64_SIZE,
                  0,
                  0,
                  sizeof(Elf64_Shdr),
                  SECTIONS.count,
                  SECTIONS.count - 1};

  fwrite(&h, sizeof(h), 1, out);
  return 1;
}

size_t getLastNonGlobal(void) {
  size_t ind = 0;
  for (size_t i = 0; i < SYMBOLS.count; i++) {
    if ((SYMBOLS.items[i].st_info & (STB_GLOBAL << 4)) == 0) {
      ind = i;
    }
  }
  return ind;
}

void align(void) {
  for (size_t i = 1; i < SECTIONS.count; i++) {
    if ((SECTIONS.items[i].sh_type & 0xf) == SHT_PROGBITS) {
      SECTIONS.items[i].sh_addralign = ARM_INSTRUCTION_SIZE;
    } else if ((SECTIONS.items[i].sh_type & 0xf) == SHT_SYMTAB) {
      SECTIONS.items[i].sh_addralign = 8;
    } else {
      SECTIONS.items[i].sh_addralign = 1;
    }
  }
}

// all entries in SECTIONS must be backpatched
u8 writeTables(FILE *out) {
  size_t symtab_size = sizeof(*SYMBOLS.items) * SYMBOLS.count;
  size_t strtab_size = getStringTableSize(SYMBOLS.strtab);
  size_t sh_info = getLastNonGlobal() + 1;

  addToShdr(".symtab", SHT_SYMTAB, 0, getSectionsEnd(), SECTIONS.count + 1,
            sh_info, sizeof(*SYMBOLS.items));
  SECTIONS.items[SECTIONS.count - 1].sh_size = symtab_size;

  addToShdr(".strtab", SHT_STRTAB, 0, getSectionsEnd(), 0, 0, 0);
  SECTIONS.items[SECTIONS.count - 1].sh_size = strtab_size;

  addToShdr(".shstrtab", SHT_STRTAB, 0, getSectionsEnd(), 0, 0, 0);
  size_t shstrtab_size = getStringTableSize(SECTIONS.shstrtab);
  SECTIONS.items[SECTIONS.count - 1].sh_size = shstrtab_size;

  align();

  fwrite(SYMBOLS.items, sizeof(*SYMBOLS.items), SYMBOLS.count, out);
  fwrite(SYMBOLS.strtab, strtab_size, 1, out);
  fwrite(SECTIONS.shstrtab, shstrtab_size, 1, out);
  fwrite(SECTIONS.items, sizeof(*SECTIONS.items), SECTIONS.count, out);
  return 1;
}

// remove duplicates and sort
u8 removeDuplicates(void) {
  Elf64_Shdr *items = SECTIONS.items;
  for (size_t i = 1; i < SECTIONS.count - 1; i++) {
    for (size_t j = i + 1; j < SECTIONS.count; j++) {
      if (items[i].sh_name == items[j].sh_name) {
        if (items[i].sh_flags != items[j].sh_flags) {
          // error here
          return 0;
        }
        // add size of second section to first section
        items[i].sh_size += items[j].sh_size;
        // move section after first section
        items[i + 1].sh_offset += items[j].sh_size;

        // remove duplicate
        for (size_t k = j; k < SECTIONS.count - 1; k++) {
          memmove(items + k, items + k + 1, sizeof(*items));
        }
        SECTIONS.count--;
      }
    }
  }

  return 1;
}

void sortSections(void) {
  Elf64_Shdr *items = SECTIONS.items;
  ssize_t order[] = {searchInShdr(".text"), searchInShdr(".data"),
                     searchInShdr(".bss")};

  u8 cur = 1;
  for (size_t i = 0; i < sizeof(order) / sizeof(*order); i++) {
    if (order[i] != -1) {
      Elf64_Shdr tmp = items[cur];
      items[cur] = items[order[i]];
      items[order[i]] = tmp;
      cur++;
    }
  }

  SECTIONS.items[1].sh_offset = ELF64_SIZE;
}

void backpatch(size_t pc_end) {
  Elf64_Shdr *section = SECTIONS.items;
  for (size_t i = 1; i < SECTIONS.count - 1; i++) {
    section[i].sh_size = section[i + 1].sh_offset - section[i].sh_offset;
  }

  section[SECTIONS.count - 1].sh_size =
      pc_end - section[SECTIONS.count - 1].sh_offset + ELF64_SIZE;

  Elf64_Sym *symbol = SYMBOLS.items;
  for (size_t i = 1; i < SECTIONS.count; i++) {
    for (size_t j = 1; j < SYMBOLS.count; j++) {
      if (symbol[j].st_value + ELF64_SIZE <= section[i].sh_offset) {
        symbol[j].st_shndx = i;
      }
    }
  }
}

u8 writeElf(FILE *out, size_t pc_end) {
  if (SECTIONS.count == 1) { // if no section default to .text
    addToShdr(".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR, 0, 0, 0, 0);
  }

  if (!removeDuplicates()) {
    return 0;
  }
  sortSections();
  backpatch(pc_end);
  writeTables(out);

  fseek(out, 0, SEEK_SET);
  writeHeader(out);
  return 1;
}
