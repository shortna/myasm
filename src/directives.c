#include "types.h"
#include <string.h>

u8 dGlobal(Fields *f) {
  (void)f;
  return 0;
}
u8 dSection(Fields *f) {
  (void)f;
  return 0;
}

typedef struct Directive {
  const char *name;
  u8 (*directive) (Fields *f);
} Directive;

Directive DIRECTIVES[] = {
  {"GLOBAL", &dGlobal},
  {"SECTION", &dSection},
};

u8 searchDirective(const char *name) {
  for (size_t i = 0; i < sizeof(DIRECTIVES) / sizeof(*DIRECTIVES); i++) {
    if (strcmp(DIRECTIVES[i].name, name) == 0) {
      return 1;
    }
  }
  return 0;
}

u8 execDirective(Fields *f) {
  (void)f;
  return 0;
}
