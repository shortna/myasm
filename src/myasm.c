#include "myasm.h"
#include "lexer/lexer.h"
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

DLA sym_table = {0};

char *ISA = NULL;
size_t ISA_LEN = 0;

void DLAInit(size_t capacity) {
  sym_table.capacity = capacity;
  sym_table.len = 0;
  sym_table.table = xmalloc(BUFFER_SIZE);
}

char *getLabel(size_t ind) {
  if (ind == 0) {
    return sym_table.table;
  }

  size_t j = 0;
  for (size_t i = 0; i < sym_table.len; i++) {
    if (j == ind) {
      return &sym_table.table[i];
    }
    if (sym_table.table[i] == '\0') {
      j++;
    }
  }
  return NULL;
}

void AddLabel(char *label) {
  if (sym_table.capacity == sym_table.len) {
    sym_table.table =
        realloc(sym_table.table, sym_table.capacity + BUFFER_SIZE);
  }
  if (sym_table.len == 0) {
    memcpy(sym_table.table, label, strlen(label) + 1);
  } else {
    char *last = getLabel(sym_table.len - 1);
    memcpy(last + strlen(last) + 1, label, strlen(label) + 1);
  }
  sym_table.len++;
}

void *xmalloc(size_t size) {
  void *p = malloc(size);
  if (!p) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  return p;
}

void MapISA(char *file_name) {
  struct stat st;
  if (stat(file_name, &st) == 0) {
    ISA_LEN = st.st_size;
  }

  int fd = open(file_name, O_RDONLY);
  ISA = mmap(NULL, ISA_LEN, PROT_READ, MAP_PRIVATE, fd, 0);
  close(fd);
}

void firstPass(const char *filename) {
  FILE *src = fopen(filename, "r");
  Token token = {0};
  getToken(src, NULL);

  while (getToken(NULL, &token) != -1) {
    printf("%20s, %d\n", token.value, token.type);
    memset(token.value, 0, MAX_TOKEN_SIZE);
  }

  fclose(src);
}

int main(int argc, char **argv) {
  if (argc == 1) {
    fprintf(stderr, "Please proved src");
    return EXIT_SUCCESS;
  }
  MapISA("/home/box/code/c/myasm/src/aarch64_instructions");
  DLAInit(100);

  firstPass(argv[1]);

  free(sym_table.table);
  munmap(ISA, ISA_LEN);
  return EXIT_SUCCESS;
}
