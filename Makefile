SHELL=/bin/bash

SRC_DIR := src
INCLUDE_DIR := $(SRC_DIR)/include

TARGET_DIR := build
TARGET := myasm

CFLAGS := -std=gnu11 -g -Wpedantic -Wall -Wextra -I $(INCLUDE_DIR) \
					-fsanitize=address,pointer-compare,pointer-subtract,leak,undefined,null,bounds,alignment
CC := clang

# MCL
# Make Clang Lsp
.PHONY: all clean MCL test

all: $(TARGET_DIR) $(TARGET)

$(TARGET_DIR):
	mkdir $@

$(TARGET): $(shell find $(SRC_DIR) -name *.c)
	$(CC) $^ $(CFLAGS) -o $(TARGET_DIR)/$@

MCL:
	bear -- make

test:
	@$(MAKE) $(TARGET) > /dev/null 2>&1
	@make -C test --no-print-directory

clean:
	$(RM) -r $(TARGET_DIR)
