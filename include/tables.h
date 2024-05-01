#ifndef MYASM_TABLES
#define MYASM_TABLES

#include "myasm.h"
#define ELF64_SIZE (64)

// forward declaration
struct _IO_FILE;
typedef struct _IO_FILE FILE;

void initSymbolTable(void);
void initShdrTable(void);

void freeSymoblTable(void);
void freeShdrTable(void);

u8 addToShdr(const char *name, u32 sh_type, u64 sh_flags, u64 sh_offset,
             u32 sh_link, u32 sh_info, u64 sh_entsize);
u8 addToSym(const char *name, u64 st_value, u64 st_size, u8 st_info);

ssize_t searchInSym(const char *needle);
ssize_t searchInShdr(const char *needle);

ssize_t getLabelPc(const char *needle);
u8 changeBinding(size_t ind, u8 binding);

u8 writeElf(FILE *out, size_t pc_end);

#endif
