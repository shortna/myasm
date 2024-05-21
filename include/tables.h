#ifndef MYASM_TABLES
#define MYASM_TABLES

#include "myasm.h"
#define ELF64_HEADER_SIZE (64)

// forward declaration
struct _IO_FILE;
typedef struct _IO_FILE FILE;

void initSymbolTable(void);
void initShdrTable(void);
void initRelocationTable(void);

void freeSymoblTable(void);
void freeShdrTable(void);
void freeRelocationTable(void);

u8 addToShdr(const char *name, u32 sh_type, u64 sh_flags, u64 sh_offset,
             u32 sh_link, u32 sh_info, u64 sh_entsize);
u8 addToSym(const char *name, u64 st_value, u64 st_size, u8 st_info, u8 st_shndx);
u8 addRelocation(const char *label, u64 info);

i64 searchInSym(const char *needle);
i64 searchInShdr(const char *needle);

i16 getLabelSection(const char *needle);
i64 getLabelPc(const char *needle);
u8 changeBinding(u64 ind, u8 binding);

u8 writeElf(FILE *out);

#endif
