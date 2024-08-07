#ifndef MYASM_MAIN
#define MYASM_MAIN

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

void *xmalloc(size_t size);
void *xrealloc(void *p, size_t size);

#endif
