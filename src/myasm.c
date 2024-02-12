#include "myasm.h"
#include "lexer/lexer.h"
#include <elf.h>
#include <string.h>
#define TABLE_SIZE (128)
#define ERROR_SIZE (255)
#define ELF_SIZE (64)

// Error stuff
typedef struct Error {
  size_t line;
  char msg[ERROR_SIZE];
} Error;

// building binary stuff
typedef struct SymbolTable {
  size_t len;       // len = n - 1, where n is number of \0
  char *names;      // label names
  Elf64_Sym *table; // labels info
} SymbolTable;

typedef struct SectionHeader {
  size_t len;        // len = n - 1, where n is number of \0
  char *names;       // section names
  Elf64_Shdr *table; // sections info
} SectionHeader;

static Elf64_Ehdr elf_header = {0};

static SymbolTable symbol_table = {0};
static SectionHeader section_header = {0};
static size_t current_section = 0; // index of current section in section_header

static Error *errors = NULL;
static size_t n_errors = 0;

static size_t LC = 0; // location counter

void *xmalloc(size_t size);
void firstPass(const char *filename);
void printError(const Error *err);
void addToSymbolTable(const char *name, u8 st_info, u8 st_other, u16 st_shndx);
void addToSectionHeader(const char *name, u32 sh_type, u64 sh_flags,
                        u64 sh_size, u32 sh_link, u32 sh_info, u64 sh_addralign,
                        u64 sh_entsize);

#define initTable(Table)                                                       \
  do {                                                                         \
    Table.table = xmalloc(sizeof(*Table.table) * TABLE_SIZE);                  \
    Table.names = xmalloc(MAX_TOKEN_SIZE * TABLE_SIZE);                        \
    Table.len = 0;                                                             \
  } while (0);

#define searchInTable(Table, name)                                             \
  ({                                                                           \
    i8 res = 0;                                                                \
    char *names = Table.names;                                                 \
    for (size_t i = 0; i < Table.len; i++) {                                   \
      if (strcmp(names, name) == 0) {                                          \
        res = i;                                                               \
      }                                                                        \
      names += strlen(names) + 1;                                              \
    }                                                                          \
    res;                                                                       \
  })

int main(int argc, char **argv) {
  if (argc == 1) {
    fprintf(stderr, "Please, provide source file");
    return EXIT_SUCCESS;
  }

  errors = xmalloc(sizeof(*errors) * TABLE_SIZE);

  initTable(symbol_table);
  initTable(section_header);

  firstPass(argv[1]);
  return EXIT_SUCCESS;
}

void printError(const Error *err) {}

void addToSymbolTable(const char *name, u8 st_info, u8 st_other, u16 st_shndx) {
  /*  Elf64_Sym *st = &symbol_table.table[symbol_table.len];
   *  st->st_name = symbol_table.len;
   *  st->st_value = LC;
   *  st->st_size = 0;
   *  st->st_info = st_info;
   *  st->st_other = st_other;
   *  st->st_shndx = st_shndx;
   *  symbol_table.len++;
   */
}

void addToSectionHeader(const char *name, u32 sh_type, u64 sh_flags,
                        u64 sh_size, u32 sh_link, u32 sh_info, u64 sh_addralign,
                        u64 sh_entsize) {
  /*  Elf64_Shdr *shdr = &section_header.table[section_header.len];
   *  shdr->sh_name = section_header.len;
   *  shdr->sh_type = sh_type;
   *  shdr->sh_flags = sh_flags;
   *  shdr->sh_addr = 0;
   *  shdr->sh_offset = LC + ELF_SIZE;
   *  shdr->sh_size = sh_size;
   *  shdr->sh_link = sh_link;
   *  shdr->sh_info = sh_info;
   *  shdr->sh_addralign = sh_addralign;
   *  shdr->sh_entsize = sh_entsize;
   */
}

void firstPass(const char *filename) {
  FILE *src = fopen(filename, "r");
  if (!src) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  getToken(src, NULL); // sets default file
  Token token = {0};

  while (getToken(NULL, &token) != EOF) {
  }

  fclose(src);
}

void *xmalloc(size_t size) {
  void *p = malloc(size);
  if (!p) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  return p;
}
