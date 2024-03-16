#include "tables.h"

#define TABLE_START_CAPACITY (10)
#define STR_START_CAPACITY UINT8_MAX

#define initTable(table)                                                       \
  do {                                                                         \
    table.capacity = TABLE_START_CAPACITY;                                     \
    table.count = 0;                                                           \
    table.items = xmalloc(table.capacity * sizeof(*table.items));              \
    table.str = initStr();                                                     \
  } while (0);

#define freeTable(table)                                                       \
  do {                                                                         \
    free(table.str.s);                                                         \
    free(table.items);                                                         \
  } while (0);

#define resizeTable(table)                                                     \
  do {                                                                         \
    table.capacity *= 2;                                                       \
    table.items =                                                              \
        xrealloc(table.items, sizeof(*table.items) * table.capacity);          \
  } while (0);

// usage:
// bool res;
// searchInTable(table, needle, res)
#define searchInTable(table, needle, res)                                      \
  do {                                                                         \
    res = 0;                                                                   \
    char *__s = table.str.s + 1;                                               \
    size_t __i = 1;                                                            \
    while (__i < table.count) {                                                \
      if (strcmp(__s, needle) == 0) {                                          \
        res = 1;                                                               \
        break;                                                                 \
      }                                                                        \
      __s += strlen(__s) + 1;                                                  \
      __i++;                                                                   \
    }                                                                          \
  } while (0);

typedef struct Str {
  char *s;
  size_t len;
  size_t capacity;
} Str;

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

static const Elf64_Shdr TEXT = {};
static const Elf64_Shdr DATA = {};
static const Elf64_Shdr BSS = {};

SymTable SYMBOLS = {0};
ShdrTable SECTIONS = {0};

Str initStr(void) {
  Str s = {0};
  s.len = 0;
  s.capacity = STR_START_CAPACITY;
  s.s = xmalloc(s.capacity * sizeof(*s.s));
  *s.s = '\0';
  return s;
}

void resizeStr(Str *str) {
  str->capacity = str->capacity * 2;
  str->s = xrealloc(str->s, sizeof(*str->s) * str->capacity);
}

void concatStr(Str *str1, const Str *str2) {
  if (str1->len + str2->len >= str1->capacity) {
    resizeStr(str1);
  }
  memcpy(str1->s + str1->len, str2->s, str2->len + 1);
  str1->len = str1->len + str2->len;
}

void concatCStr(Str *str1, const char *str2) {
  size_t str2_len = strlen(str2);
  if (str1->len + str2_len >= str1->capacity) {
    resizeStr(str1);
  }
  memcpy(str1->s + str1->len, str2, str2_len + 1);
  str1->len = str1->len + str2_len;
}

// st_size depends on type of label
// if label has format - label:
// st_size = 0
// if label has format - label = ...
// st_size needs to be calculated
//
// st_info = STT_NOTYPE and type of binding e.g STB_LOCAL or STB_GLOBAL
u8 addToSym(const char *name, Elf64_Addr st_value, u64 st_size, u8 st_info) {

  bool res;
  searchInTable(SYMBOLS, name, res);
  if (res) {
    return ALREADY_DEFINED;
  }

  if (SYMBOLS.count == SYMBOLS.capacity) {
    resizeTable(SYMBOLS);
  }
  concatCStr(&SYMBOLS.str, name);
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

// sh_type, sh_flags - refer to man elf
// sh_offset = program counter
// sh_link = index of referd section e.g SYMTAB referers to STRTAB
// sh_info = refere to
// https://www.sco.com/developers/gabi/latest/ch4.sheader.html#sh_link
// sh_entsize = size of entry if section has entries valid only for symtab
u8 addToShdr(const char *name, u32 sh_type, u64 sh_flags, Elf64_Off sh_offset,
             u32 sh_link, u32 sh_info, u64 sh_addralign, u64 sh_entsize) {

  bool res;
  searchInTable(SECTIONS, name, res);
  if (res) {
    return ALREADY_DEFINED;
  }

  if (SECTIONS.count == SECTIONS.capacity) {
    resizeTable(SECTIONS);
  }
  concatCStr(&SECTIONS.str, name);
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

void backputchTables(size_t pc_end) {
  for (size_t i = 0; i < SECTIONS.count - 1; i++) {
    SECTIONS.items[i].sh_size =
        SECTIONS.items[i + 1].sh_size - SECTIONS.items[i].sh_size;
  }
  SECTIONS.items[SECTIONS.count - 1].sh_size =
      pc_end - SECTIONS.items[SECTIONS.count - 1].sh_size;
}

void initTables(void) {
  initTable(SECTIONS);
  initTable(SYMBOLS);
}

void freeTables(void) {
  freeTable(SYMBOLS);
  freeTable(SECTIONS);
}
