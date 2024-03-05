#include "build_binary.h"

#ifdef DEBUG
#define SRC_FILE "build_binary.c"
#endif

#define TABLE_SIZE (128)
#define RESIZE_TABLE(Table)                                                    \
  do {                                                                         \
    Table.capacity *= 2;                                                       \
    Table.table =                                                              \
        xrealloc(Table.table, sizeof(*Table.table) * Table.capacity);          \
    Table.names =                                                              \
        xrealloc(Table.names, sizeof(*Table.names) * Table.capacity);          \
  } while (0);

static Elf64_Ehdr elf_header = {0};
SymbolTable symbol_table = {0};     // symtab
SectionHeader section_header = {0}; // shstrtab

Str initStr(void) {
  Str str = {0};
  return str;
}

void initSymbolTable(void) {
  symbol_table.capacity = TABLE_SIZE;

  symbol_table.table =
      xmalloc(sizeof(*symbol_table.table) * symbol_table.capacity);
  symbol_table.names =
      xmalloc(sizeof(*symbol_table.names) * symbol_table.capacity);

  symbol_table.len = 0;
  addToSymbolTable(NULL, 0, ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE), 0, SHN_UNDEF);
}

void initSectionHeader(void) {
  section_header.capacity = TABLE_SIZE;

  section_header.table =
      xmalloc(sizeof(*section_header.table) * section_header.capacity);
  section_header.names =
      xmalloc(sizeof(*section_header.names) * section_header.capacity);

  section_header.len = 0;
  addToSectionHeader(NULL, 0, SHT_NULL, 0, 0, 0);
}

void buildHeader(void) {}

void initTables(void) {
  initSymbolTable();
  initSectionHeader();
}

// labeles declared like this: len = . - label
// have st_size
// but lexer have no capabilities to identify those labels
// fix lexer and modify this function to accept st_size
void addToSymbolTable(char *name, Elf64_Addr addr, u8 st_info, u8 st_other,
                      Elf64_Section section) {

  Elf64_Sym *t = &symbol_table.table[symbol_table.len];

  t->st_name = symbol_table.len;
  t->st_value = addr;
  t->st_size = 0;
  t->st_info = st_info;
  t->st_other = st_other;
  t->st_shndx = section;

  symbol_table.len++;

  if (symbol_table.len - 1 == symbol_table.capacity) {
    RESIZE_TABLE(symbol_table);
  }
}

void addToSectionHeader(char *name, Elf64_Word sh_type, Elf64_Xword sh_flags,
                        Elf64_Off offset, Elf64_Xword addraling,
                        Elf64_Xword sh_entsize) {

  Elf64_Shdr *t = &section_header.table[section_header.len];

  t->sh_name = section_header.len;
  t->sh_type = sh_type;
  t->sh_flags = sh_flags;
  t->sh_addr = 0; // everything is offset from zero
  t->sh_offset = offset;

  t->sh_size = 0; // must be set later

  t->sh_link = 0;
  t->sh_info = 0;

  t->sh_addralign = addraling;
  t->sh_entsize = sh_entsize;
  section_header.len++;

  if (section_header.len - 1 == section_header.capacity) {
    RESIZE_TABLE(section_header);
  }
}

void dumpTables() {
#ifdef DEBUG
#line 93 SRC_FILE
#warning "dumpTables Not Implemented"
#endif
}

void setSizes() {
  // the least amount of sections is 4:
  // .text
  // .strtab
  // .symtab
  // .shstrtab
  Elf64_Shdr *t = section_header.table;
  size_t l = section_header.len;

  for (size_t i = 0; i < l - 1; i++) {
    t[i].sh_size = t[i + 1].sh_offset - t[i].sh_offset;
  }
  t[l - 1].sh_size = t[l - 1].sh_offset - t[l - 2].sh_offset;
}
