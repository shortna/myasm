SHELL=/bin/bash

SRC_DIR := src
INCLUDE_DIR := include

TARGET_DIR := build
TARGET := myasm

CFLAGS := -std=c11 -g -Wpedantic -Wall -Wextra -I $(INCLUDE_DIR) -D DEBUG \
					-fsanitize=address,pointer-compare,pointer-subtract,leak,undefined,null,bounds,alignment

CC := clang

.PHONY: all clean test crosscompile

all: $(TARGET_DIR) $(TARGET)

$(TARGET_DIR):
	mkdir $@

$(TARGET): $(SRC_DIR)/*
	$(CC) $^ $(CFLAGS) -o $(TARGET_DIR)/$@

test:
	@$(MAKE) $(TARGET) > /dev/null 2>&1
	@$(MAKE) -C test --no-print-directory

clean:
	$(RM) -r $(TARGET_DIR)
