#include "tables.h"
#include "types.h"
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TABLE_START_CAPACITY (10)

#define initTable(table)                                                       \
  do {                                                                         \
    table.capacity = TABLE_START_CAPACITY;                                     \
    table.count = 0;                                                           \
    table.items = xmalloc(table.capacity * sizeof(*table.items));              \
    table.str = initStr();                                                     \
  } while (0)

#define freeTable(table)                                                       \
  do {                                                                         \
    free(table.str.s);                                                         \
    free(table.items);                                                         \
  } while (0)

#define resizeTable(table)                                                     \
  do {                                                                         \
    table.capacity *= 2;                                                       \
    table.items =                                                              \
        xrealloc(table.items, sizeof(*table.items) * table.capacity);          \
  } while (0)

// usage:
// int res;
// searchInTable(table, needle, res)
#define searchInTable(table, needle, res)                                      \
  do {                                                                         \
    res = -1;                                                                  \
    char *__s = table.str.s + 1;                                               \
    size_t __i = 1;                                                            \
    while (__i < table.count) {                                                \
      if (strcmp(__s, needle) == 0) {                                          \
        res = __i;                                                             \
        break;                                                                 \
      }                                                                        \
      __s += strlen(__s) + 1;                                                  \
      __i++;                                                                   \
    }                                                                          \
  } while (0)

typedef struct SymTable {
  Str str;
  Elf64_Sym *items;
  size_t count;
  size_t capacity;
} SymTable;

typedef struct ShdrTable {
  Str str;
  Elf64_Shdr *items;
  size_t count;
  size_t capacity;
} ShdrTable;

SymTable SYMBOLS = {0};
ShdrTable SECTIONS = {0};

void initTables(void) {
  initTable(SYMBOLS);
  initTable(SECTIONS);
  addToShdr(NULL, SHT_NULL, 0, 0, 0, 0, 0, 0);
  addToSym(NULL, 0, 0, 0);
}

void freeTables(void) {
  freeTable(SYMBOLS);
  freeTable(SECTIONS);
}

ssize_t searchInSym(const char *needle) {
  if (!needle) {
    return -1;
  }

  ssize_t res = -1;
  searchInTable(SYMBOLS, needle, res);
  return res;
}

ssize_t searchInShdr(const char *needle) {
  if (!needle) {
    return -1;
  }

  ssize_t res = -1;
  searchInTable(SECTIONS, needle, res);
  return res;
}

ssize_t getLabelPc(const char *needle) {
  ssize_t res = searchInSym(needle);
  if (res == -1) {
    return res;
  }
  return SYMBOLS.items[res].st_value; // return pc
}

u8 addToSym(const char *name, Elf64_Addr st_value, u64 st_size, u8 st_info) {
  ssize_t res = searchInSym(name);
  if (res != -1) {
    return 0;
  }

  if (SYMBOLS.count == SYMBOLS.capacity) {
    resizeTable(SYMBOLS);
  }
  concatCStr(&SYMBOLS.str, name);
  SYMBOLS.str.len++;
  Elf64_Sym *item = SYMBOLS.items + SYMBOLS.count;
  item->st_name = SYMBOLS.count;
  item->st_value = st_value;
  item->st_size = st_size;
  item->st_info = st_info;
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
    resizeTable(SECTIONS);
  }
  concatCStr(&SECTIONS.str, name);
  SECTIONS.str.len++;
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

// sketchy teritory
u8 changeBinding(size_t ind, u8 binding) {
  if (ind >= SYMBOLS.count) {
    return 0;
  }

  u8 current_type = SYMBOLS.items[ind].st_info & 0xf;
  SYMBOLS.items[ind].st_info = ELF64_ST_INFO(binding, current_type);
  return 1;
}

void backputchTables(size_t pc_end) {
  for (size_t i = 0; i < SECTIONS.count - 1; i++) {
    SECTIONS.items[i].sh_size =
        SECTIONS.items[i + 1].sh_size - SECTIONS.items[i].sh_size;
  }
  SECTIONS.items[SECTIONS.count - 1].sh_size =
      pc_end - SECTIONS.items[SECTIONS.count - 1].sh_size;
}

size_t START_OF_SECTION_HEADERS = 0;
void dumpTables(FILE *f) {
}

void getHeader(void *Elf_Ehdr) {
  Elf64_Ehdr h = {{EI_MAG0, EI_MAG1, EI_MAG2, EI_MAG3, ELFCLASS64, ELFDATA2LSB,
                   EV_CURRENT, ELFOSABI_SYSV, 0, 0, 0, 0, 0, 0, 0, EI_NIDENT},
                  ET_REL,
                  EM_AARCH64,
                  EV_CURRENT,
                  0,
                  0,
                  START_OF_SECTION_HEADERS,
                  0,
                  ELF64_SIZE,
                  0,
                  0,
                  sizeof(Elf64_Shdr),
                  SECTIONS.count + 3,
                  SECTIONS.count + 2};
  memcpy(Elf_Ehdr, &h, sizeof(h));
}
