#include "types.h"
#include "lexer.h"
#include <stdlib.h>
#include <string.h>

Context CONTEXT = {0};

Fields initFields(size_t size_of_field) {
  Fields f = {0};
  for (u8 i = 0; i < FIELDS_MAX; i++) {
    f.fields[i] = initToken(size_of_field);
  }
  return f;
}

void freeFields(Fields *fields) {
  if (!fields) {
    return;
  }
  for (u8 i = 0; i < FIELDS_MAX; i++) {
    free(fields->fields[i].value);
  }
}

Token initToken(size_t size) {
  Token t = {0};
  t.capacity = size;
  t.value = xmalloc(t.capacity * sizeof(*t.value));
  *t.value = '\0';
  return t;
}

void copyToken(Token *dst, const Token *src) {
  if (dst->capacity < src->capacity || !dst->value) {
    dst->capacity = src->capacity;
    dst->value = xrealloc(dst->value, dst->capacity * sizeof(*dst->value));
  }
  dst->type = src->type;
  strcpy(dst->value, src->value);
}

void resizeToken(Token *t) {
  t->capacity *= 2;
  t->value = xrealloc(t->value, t->capacity * sizeof(*t->value));
}
