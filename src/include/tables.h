#ifndef MYASM_TABLES
#define MYASM_TABLES

#include "myasm.h"
#define ELF64_SIZE (64)

typedef u64 Elf64_Addr;
typedef u64 Elf64_Off;

// forward declaration
struct _IO_FILE;
typedef struct _IO_FILE FILE;

void initTables(void);
void freeTables(void);

u8 addToShdr(const char *name, u32 sh_type, u64 sh_flags, Elf64_Off sh_offset,
             u32 sh_link, u32 sh_info, u64 sh_addralign, u64 sh_entsize);
u8 addToSym(const char *name, Elf64_Addr st_value, u64 st_size, u8 st_info);

ssize_t searchInSym(const char *needle);
ssize_t searchInShdr(const char *needle);

u8 changeBinding(size_t ind, u8 binding);
ssize_t getLabelPc(const char *needle);

void backputchTables(size_t pc_end);
void dumpTables(FILE *f);

void getHeader(void* Elf_Ehdr);

#endif
