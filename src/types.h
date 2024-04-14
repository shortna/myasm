#ifndef MYASM_TYPES
#define MYASM_TYPES

#include "myasm.h"
#define STR_START_CAPACITY UINT8_MAX

typedef struct Str {
  char *s;
  size_t len;
  size_t capacity;
} Str;


Str initStr(void);
void resizeStr(Str *str);
void concatStr(Str *str1, const Str *str2);
void concatCStr(Str *str1, const char *str2);

void toUpper(Str *line);

#endif
