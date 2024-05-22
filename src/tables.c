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
  u64 count;
  u64 capacity;
} SymTable;

typedef struct ShdrTable {
  char *shstrtab;
  Elf64_Shdr *items;
  u64 count;
  u64 capacity;
} ShdrTable;

typedef struct RelocationSection {
  Elf64_Rela *items;
  u64 count;
  u64 capacity;
  u8 shndx;
} RelocationSection;

typedef struct RelocationTable {
  RelocationSection *sections;
  u64 count;
  u64 capacity;
} RelocationTable;

static SymTable SYMBOLS = {0};
static ShdrTable SECTIONS = {0};
static RelocationTable RELOCATIONS = {0};

void initSymbolTable(void) {
  SYMBOLS.items = xmalloc(TALBE_START_SIZE * sizeof(*SYMBOLS.items));
  SYMBOLS.strtab =
      xcalloc(TALBE_START_SIZE, TOKEN_SIZE * sizeof(*SYMBOLS.strtab));

  SYMBOLS.count = 0;
  SYMBOLS.capacity = TALBE_START_SIZE;
  addToSym("", 0, 0, 0, 0);
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

void initRelocationTable(void) {
  RELOCATIONS.capacity = TALBE_START_SIZE;
  RELOCATIONS.sections =
      xmalloc(RELOCATIONS.capacity * sizeof(*RELOCATIONS.sections));
  for (u8 i = 0; i < TALBE_START_SIZE; i++) {
    RELOCATIONS.sections[i].items =
        xmalloc(TALBE_START_SIZE * sizeof(*RELOCATIONS.sections->items));
    RELOCATIONS.sections[i].capacity = TALBE_START_SIZE;
    RELOCATIONS.sections[i].count = 0;
    RELOCATIONS.sections[i].shndx = 0;
  }
}

void freeSymoblTable(void) {
  free(SYMBOLS.items);
  free(SYMBOLS.strtab);
}

void freeShdrTable(void) {
  free(SECTIONS.items);
  free(SECTIONS.shstrtab);
}

void freeRelocationTable(void) {
  for (u64 i = 0; i < RELOCATIONS.capacity; i++) {
    free(RELOCATIONS.sections[i].items);
  }
  free(RELOCATIONS.sections);
}

i64 searchTable(const char *needle, const char *strtable, u64 count) {
  strtable++; // skip first entry cause its always \0
  u64 i = 1;
  while (i < count) {
    if (strcmp(needle, strtable) == 0) {
      return i;
    }
    strtable += strlen(strtable) + 1;
    i++;
  }
  return -1;
}

i64 searchInSym(const char *needle) {
  if (!needle) {
    return -1;
  }

  i64 res = searchTable(needle, SYMBOLS.strtab, SYMBOLS.count);
  return res;
}

i64 searchInShdr(const char *needle) {
  if (!needle) {
    return -1;
  }

  i64 res = searchTable(needle, SECTIONS.shstrtab, SECTIONS.count);
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

void resizeRelocationsTable(void) {
  RELOCATIONS.capacity *= 2;
  RELOCATIONS.sections = xrealloc(RELOCATIONS.sections,
                                  RELOCATIONS.capacity * sizeof(RELOCATIONS));
}

void resizeRelocationsSection(void) {
  u64 cur = RELOCATIONS.count - 1;
  RELOCATIONS.sections[cur].capacity *= 2;
  RELOCATIONS.sections[cur].items =
      xrealloc(RELOCATIONS.sections[cur].items,
               RELOCATIONS.sections[cur].capacity *
                   sizeof(*RELOCATIONS.sections->items));
}

i64 getLabelPc(const char *needle) {
  i64 res = searchInSym(needle);
  if (res == -1) {
    return res;
  }
  return SYMBOLS.items[res].st_value; // return pc
}

i16 getLabelSection(const char *needle) {
  i64 res = searchInSym(needle);
  if (res == -1) {
    return res;
  }
  return SYMBOLS.items[res].st_shndx; // return section index
}

// concats s to table with delemiter between '\0' them
u8 concatStrs(char *table, const char *s, u64 count) {
  table++;
  u64 i = 1;
  while (i < count) {
    table += strlen(table) + 1;
    i++;
  }

  strncat(table, s, TOKEN_SIZE - 1);
  return 1;
}

u64 getStringTableSize(const char *table);

u8 addToSym(const char *name, Elf64_Addr st_value, u64 st_size, u8 st_info,
            u8 st_shndx) {
  i64 res = searchInSym(name);
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
  item->st_shndx = st_shndx;
  concatStrs(SYMBOLS.strtab, name, SYMBOLS.count);
  SYMBOLS.count++;
  return 1;
}

u8 addToShdr(const char *name, u32 sh_type, u64 sh_flags, Elf64_Off sh_offset,
             u32 sh_link, u32 sh_info, u64 sh_entsize) {

  i64 res = searchInShdr(name);
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

u8 addRelocation(const char *label, u64 info) {
  if (RELOCATIONS.count == RELOCATIONS.capacity) {
    resizeRelocationsTable();
  }

  i64 label_ind = searchInSym(label);
  if (label_ind == -1) {
    return 0;
  }

  u64 i = 0;
  while (i < RELOCATIONS.count) {
    if (RELOCATIONS.sections[i].shndx == CONTEXT.cur_sndx) {
      break;
    }
    i++;
  }

  if (i == RELOCATIONS.count) {
    RELOCATIONS.count++;
  }

  RelocationSection *section = RELOCATIONS.sections + i;
  if (section->count == section->capacity) {
    resizeRelocationsSection();
  }

  Elf64_Rela *item = section->items + section->count;

//  item->r_addend = SYMBOLS.items[label_ind].st_value;
  item->r_addend = 0;
  item->r_offset = CONTEXT.pc;
  item->r_info = ELF64_R_INFO(label_ind, info);

  section->count++;
  section->shndx = CONTEXT.cur_sndx;
  return 1;
}

u8 changeBinding(u64 ind, u8 binding) {
  if (ind >= SYMBOLS.count) {
    return 0;
  }

  u8 current_type = SYMBOLS.items[ind].st_info & 0xf;
  SYMBOLS.items[ind].st_info = ELF64_ST_INFO(binding, current_type);
  return 1;
}

u64 getStringTableSize(const char *table) {
  const char *table_s = table;
  table++;
  while (*table) {
    table += strlen(table) + 1;
  }
  return table - table_s;
}

u64 getSectionsEnd(void) {
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
                  ELF64_HEADER_SIZE,
                  0,
                  0,
                  sizeof(Elf64_Shdr),
                  SECTIONS.count,
                  SECTIONS.count - 1};

  fwrite(&h, sizeof(h), 1, out);
  return 1;
}

u64 getLastNonGlobal(void) {
  u64 ind = 0;
  for (u64 i = 0; i < SYMBOLS.count; i++) {
    if ((SYMBOLS.items[i].st_info & (STB_GLOBAL << 4)) == 0) {
      ind = i;
    }
  }
  return ind;
}

void align(void) {
  for (u64 i = 1; i < SECTIONS.count; i++) {
    u64 type = SECTIONS.items[i].sh_type & 0xf;
    if (type == SHT_PROGBITS) {
      SECTIONS.items[i].sh_addralign = ARM_INSTRUCTION_SIZE;
    } else if (type == SHT_SYMTAB || type == SHT_RELA) {
      SECTIONS.items[i].sh_addralign = 8;
    } else {
      SECTIONS.items[i].sh_addralign = 1;
    }
  }
}

void dumpRelocations(void) {
  char name[TOKEN_SIZE];
  for (u64 i = 0; i < RELOCATIONS.count; i++) {
    sprintf(name, ".rela%lu", i);
    addToShdr(name, SHT_RELA, SHF_INFO_LINK, getSectionsEnd(),
              SECTIONS.count + 1, RELOCATIONS.sections[i].shndx,
              sizeof(*RELOCATIONS.sections->items));
    SECTIONS.items[SECTIONS.count - 1].sh_size =
        sizeof(*RELOCATIONS.sections->items) * RELOCATIONS.sections[i].count;
  }
}

void backpatch(void) {
  Elf64_Shdr *section = SECTIONS.items;
  for (u8 i = 1; i < SECTIONS.count - 1; i++) {
    section[i].sh_size = section[i + 1].sh_offset - section[i].sh_offset;
  }

  section[SECTIONS.count - 1].sh_size =
      CONTEXT.size - section[SECTIONS.count - 1].sh_offset;
}

// swaps left with right
void swapInSym(u64 left, u64 right) {
  for (u64 i = 0; i < RELOCATIONS.count; i++) {
    for (u64 j = 0; j < RELOCATIONS.sections[i].count; j++) {
      u64 ind = ELF64_R_SYM(RELOCATIONS.sections[i].items[j].r_info);
      u64 type = ELF64_R_TYPE(RELOCATIONS.sections[i].items[j].r_info);
      if (ind == left) {
        RELOCATIONS.sections[i].items[j].r_info = ELF64_R_INFO(right, type);
      } else if (ind == right) {
        RELOCATIONS.sections[i].items[j].r_info = ELF64_R_INFO(left, type);
      }
    }
  }
  Elf64_Sym right_copy = SYMBOLS.items[right];
  SYMBOLS.items[right] = SYMBOLS.items[left];
  SYMBOLS.items[left] = right_copy;
}

void sortSymbols(void) {
  u8 last = 0;
  i64 start = searchInSym("_start");
  if (start != -1 &&
      ELF64_ST_BIND(SYMBOLS.items[start].st_info) == STB_GLOBAL &&
      (u64)start != SYMBOLS.count - 1) {
    swapInSym(start, SYMBOLS.count - 1);
    last = 1;
  }

  u64 len = SYMBOLS.count - last - 1;
  u64 i = 0, end = len;
  while (i < end) {
    while (ELF64_ST_BIND(SYMBOLS.items[end].st_info) == STB_GLOBAL && i < end) {
      end--;
    }

    if (i == end) {
      break;
    }

    if (ELF64_ST_BIND(SYMBOLS.items[i].st_info) == STB_GLOBAL) {
      swapInSym(i, end);
    }
    i++;
    end--;
  }
}

// all entries in SECTIONS must be backpatched
u8 writeTables(FILE *out) {
  sortSymbols();
  dumpRelocations();
  u64 symtab_size = sizeof(*SYMBOLS.items) * SYMBOLS.count;
  u64 strtab_size = getStringTableSize(SYMBOLS.strtab);
  u64 sh_info = getLastNonGlobal() + 1;

  addToShdr(".symtab", SHT_SYMTAB, 0, getSectionsEnd(), SECTIONS.count + 1,
            sh_info, sizeof(*SYMBOLS.items));
  SECTIONS.items[SECTIONS.count - 1].sh_size = symtab_size;

  addToShdr(".strtab", SHT_STRTAB, 0, getSectionsEnd(), 0, 0, 0);
  SECTIONS.items[SECTIONS.count - 1].sh_size = strtab_size;

  addToShdr(".shstrtab", SHT_STRTAB, 0, getSectionsEnd(), 0, 0, 0);
  u64 shstrtab_size = getStringTableSize(SECTIONS.shstrtab);
  SECTIONS.items[SECTIONS.count - 1].sh_size = shstrtab_size;

  align();

  for (u8 i = 1; i < SECTIONS.count; i++) {
    SECTIONS.items[i].sh_offset += ELF64_HEADER_SIZE;
  }

  for (u64 i = 0; i < RELOCATIONS.count; i++) {
    fwrite(RELOCATIONS.sections[i].items, sizeof(*RELOCATIONS.sections->items),
           RELOCATIONS.sections[i].count, out);
  }

  fwrite(SYMBOLS.items, sizeof(*SYMBOLS.items), SYMBOLS.count, out);
  fwrite(SYMBOLS.strtab, strtab_size, 1, out);
  fwrite(SECTIONS.shstrtab, shstrtab_size, 1, out);
  fwrite(SECTIONS.items, sizeof(*SECTIONS.items), SECTIONS.count, out);
  return 1;
}

u8 writeElf(FILE *out) {
  if (SECTIONS.count == 1) {
    return 0;
  }
  backpatch();
  writeTables(out);
  fseek(out, 0, SEEK_SET);
  writeHeader(out);
  return 1;
}
