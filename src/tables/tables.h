#ifndef MYASM_TABLES_API
#define MYASM_TABLES_API

#include "types.h"
#include <elf.h>
#define ELF64_SIZE (64)

void initTables(void);
void freeTables(void);

u8 addToShdr(const char *name, u32 sh_type, u64 sh_flags, Elf64_Off sh_offset,
             u32 sh_link, u32 sh_info, u64 sh_addralign, u64 sh_entsize);
u8 addToSym(const char *name, Elf64_Addr st_value, u64 st_size, u8 st_info);

ssize_t searchInSym(const char *needle);
ssize_t searchInShdr(const char *needle);

ssize_t getLabelPc(const char *needle);
Elf64_Ehdr getHeader();

#endif
