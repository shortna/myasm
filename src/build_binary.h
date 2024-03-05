#ifndef MYASM_BUILDB
#define MYASM_BUILDB

#include "myasm.h"
#include <elf.h>

#define ELF64_SIZE (64)

typedef struct Str {
  char str[MAX_TOKEN_SIZE];
  u8 len;
} Str;

typedef struct SymbolTable { // symtab
  Str *names;
  size_t len;
  size_t capacity;
  Elf64_Sym *table; // labels info
} SymbolTable;

typedef struct SectionHeader { // shstrtab
  Str *names;
  size_t len;
  size_t capacity;
  Elf64_Shdr *table; // sections info
} SectionHeader;

void buildElfHeader(void);
void initTables(void);

void addToSymbolTable(char *name, Elf64_Addr addr, u8 st_info, u8 st_other,
                      Elf64_Section section);
void addToSectionHeader(char *name, Elf64_Word sh_type, Elf64_Xword sh_flags,
                        Elf64_Off offset, Elf64_Xword addraling,
                        Elf64_Xword sh_entsize);

void dumpTables();

// must be called after dumpTables()
void setSizes();

#endif
