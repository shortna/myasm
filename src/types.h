#ifndef MYASM_TYPES
#define MYASM_TYPES

#include "myasm.h"
#define STR_START_CAPACITY UINT8_MAX
#define FIELDS_MAX (7)

typedef struct Str {
  char *s;
  size_t len;
  size_t capacity;
} Str;

typedef struct fields_t {
  Str fields[FIELDS_MAX]; // change this to STR api
  u8 n_fields;
} fields_t;

Str initStr(void);
void resizeStr(Str *str);
void concatStr(Str *str1, const Str *str2);
void concatCStr(Str *str1, const char *str2);

fields_t initFields(void);
void freeFields(fields_t *fields);

#endif
