#ifndef MYASM
#define MYASM

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#define BUFFER_SIZE 1024

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef struct DynamicLabelsArray {
  char *table;
  size_t len;
  size_t capacity;
} DLA;

extern char* ISA;
extern size_t ISA_LEN;
extern DLA sym_table;

void *xmalloc(size_t size);
void AddLabel(char *label);
char *getLabel(size_t ind);

#endif
