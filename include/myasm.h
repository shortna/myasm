#ifndef MYASM_MAIN
#define MYASM_MAIN

#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef alloca
#define alloca(size) __builtin_alloca(size)
#endif

#define TOKEN_SIZE (40)
#define FIELDS_SIZE (10)

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

struct Fields;
typedef struct Fields Fields;

struct Token;
typedef struct Token Token;

extern u64 LINE;
extern bool ERRORS;

void errorFields(const char *msg, const Fields *f); 
void errorToken(const char *msg, const Token *t); 
void error(const char *msg); 

void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *p, size_t size);

#endif
