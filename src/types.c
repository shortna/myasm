#include "types.h"

Str initStr(void) {
  Str s = {0};
  s.len = 0;
  s.capacity = STR_START_CAPACITY;
  s.s = xmalloc(s.capacity * sizeof(*s.s));
  *s.s = '\0';
  return s;
}

void resizeStr(Str *str) {
  str->capacity = str->capacity * 2;
  str->s = xrealloc(str->s, sizeof(*str->s) * str->capacity);
}

void concatStr(Str *str1, const Str *str2) {
  if (!str1 || !str2) {
    return;
  }

  if (str1->len + str2->len >= str1->capacity) {
    resizeStr(str1);
  }
  memcpy(str1->s + str1->len, str2->s, str2->len + 1);
  str1->len = str1->len + str2->len;
}

void concatCStr(Str *str1, const char *str2) {
  if (!str1 || !str2) {
    return;
  }

  size_t str2_len = strlen(str2);
  if (str1->len + str2_len >= str1->capacity) {
    resizeStr(str1);
  }
  memcpy(str1->s + str1->len, str2, str2_len + 1);
  str1->len = str1->len + str2_len;
}

fields_t initFields(void) {
  fields_t f = {0};
  for (u8 i = 0; i < FIELDS_MAX; i++) {
    f.fields[i] = initStr();
  }
  return f;
}

void freeFields(fields_t *fields) {
  if (!fields) {
    return;
  }
  for (u8 i = 0; i < FIELDS_MAX; i++) {
    free(fields->fields[i].s);
  }
}
