#include "tables.h"
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
}

void initShdrTable(void) {
  SECTIONS.items = xmalloc(TALBE_START_SIZE * sizeof(*SECTIONS.items));
  SECTIONS.shstrtab =
      xcalloc(TALBE_START_SIZE, TOKEN_SIZE * sizeof(*SECTIONS.shstrtab));

  SECTIONS.count = 0;
  SECTIONS.capacity = TALBE_START_SIZE;
  addToShdr("", SHT_NULL, 0, 0, 0, 0, 0, 0);
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

u8 addToSym(const char *name, Elf64_Addr st_value, u64 st_size, u8 st_info) {
  ssize_t res = searchInSym(name);
  if (res != -1) {
    return 0;
  }

  if (SYMBOLS.count == SYMBOLS.capacity + 1) {
    resizeSymTable();
  }
  concatStrs(SYMBOLS.strtab, name, SYMBOLS.count);
  Elf64_Sym *item = SYMBOLS.items + SYMBOLS.count;
  item->st_name = SYMBOLS.count;
  item->st_value = st_value;
  item->st_size = st_size;
  item->st_info = ELF64_ST_INFO(STB_LOCAL, st_info);
  item->st_other = STV_DEFAULT;
  item->st_shndx = SECTIONS.count - 1;
  SYMBOLS.count++;
  return 1;
}

u8 addToShdr(const char *name, u32 sh_type, u64 sh_flags, Elf64_Off sh_offset,
             u32 sh_link, u32 sh_info, u64 sh_addralign, u64 sh_entsize) {

  ssize_t res = searchInShdr(name);
  if (res != -1) {
    return 0;
  }

  if (SECTIONS.count == SECTIONS.capacity) {
    resizeShdrTable();
  }
  concatStrs(SECTIONS.shstrtab, name, SECTIONS.count);
  Elf64_Shdr *item = SECTIONS.items + SECTIONS.count;
  item->sh_name = SECTIONS.count;
  item->sh_type = sh_type;
  item->sh_flags = sh_flags;
  item->sh_addr = 0;
  item->sh_offset = sh_offset;
  item->sh_size = 0;
  item->sh_link = sh_link;
  item->sh_info = sh_info;
  item->sh_addralign = sh_addralign;
  item->sh_entsize = sh_entsize;
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

// FIX THIS SHIT
size_t getEnd(void) {
  size_t end = SECTIONS.items[SECTIONS.count - 1].sh_offset +
               SECTIONS.items[SECTIONS.count - 1].sh_size;
  //  if ((end & 1) != 0) {
  //    end += 1;
  //  }

  return end;
}

void backputchSections(size_t pc_end) {
  Elf64_Shdr *s = SECTIONS.items;
  for (size_t i = 0; i < SECTIONS.count - 1; i++) {
    s[i].sh_size = s[i + 1].sh_offset - s[i].sh_offset;
  }
  s[SECTIONS.count - 1].sh_size = pc_end - s[SECTIONS.count - 1].sh_offset;
}

size_t getStrTableSize(const char *table, size_t count) {
  const char *start = table;
  size_t i = 1;
  while (i < count) {
    table += strlen(table) + 1;
    i++;
  }

  return table - start;
}

void dumpTables(void) {
  Elf64_Shdr *last = SECTIONS.items + SECTIONS.count - 1;
  addToShdr(".SYMTAB", SHT_SYMTAB, 0, getEnd(), SECTIONS.count, SYMBOLS.count,
            8, sizeof(*SYMBOLS.items));
  last++;
  last->sh_size = SYMBOLS.count * sizeof(*SYMBOLS.items);

  addToShdr(".STRTAB", SHT_STRTAB, 0, getEnd(), 0, 0, 1, 0);
  last++;
  last->sh_size = getStrTableSize(SYMBOLS.strtab, SYMBOLS.count);

  addToShdr(".SHSTRTAB", SHT_STRTAB, 0, getEnd(), 0, 0, 1, 0);
  last++;
  last->sh_size = getStrTableSize(SECTIONS.shstrtab, SECTIONS.count);
}

u8 writeTables(FILE *out, size_t pc_end) {
  backputchSections(pc_end);
  dumpTables();

  fwrite(SYMBOLS.items, sizeof(*SYMBOLS.items), SYMBOLS.count,
         out); // write symtable

  fwrite(SYMBOLS.strtab, getStrTableSize(SYMBOLS.strtab, SYMBOLS.count), 1,
         out);
  fwrite(SECTIONS.shstrtab, getStrTableSize(SECTIONS.shstrtab, SECTIONS.count),
         1, out);

  fwrite(SECTIONS.items, sizeof(*SECTIONS.items), SECTIONS.count, out);

  return 1;
}
// FIX THIS SHIT

u8 writeHeader(FILE *out) {
  Elf64_Off e_shoff = getEnd();
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
