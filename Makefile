SHELL=/bin/bash

SRC_DIR := src

TARGET_DIR := build
TARGET := myasm

CFLAGS := -std=gnu11 -g -Wpedantic -Wall -Wextra -fsanitize=address,pointer-compare,pointer-subtract,leak,undefined,null,bounds,alignment -D DEBUG

.PHONY: all clean

all: $(TARGET_DIR) $(TARGET)

$(TARGET_DIR):
	mkdir $@

$(TARGET): $(shell find $(SRC_DIR) -name *.c)
	$(CC) $^ $(CFLAGS) -o $(TARGET_DIR)/$@

clean:
	$(RM) -r $(TARGET_DIR)
