SHELL=/bin/bash

SRC_DIR := src

TARGET_DIR := build
TARGET := myasm

CFLAGS := -std=gnu11 -g -Wpedantic -Wall -Wextra -I $(SRC_DIR) \
					-fsanitize=address,pointer-compare,pointer-subtract,leak,undefined,null,bounds,alignment
CC := clang

# MCL
# Make Clang Lsp
.PHONY: all clean MCL

all: $(TARGET_DIR) $(TARGET)

$(TARGET_DIR):
	mkdir $@

$(TARGET): $(shell find $(SRC_DIR) -name *.c)
	$(CC) $^ $(CFLAGS) -o $(TARGET_DIR)/$@

MCL:
	bear -- make

clean:
	$(RM) -r $(TARGET_DIR)
