SHELL=/bin/bash

SRC_DIR := src
INCLUDE_DIR := include

TARGET_DIR := build
TARGET := myasm

CFLAGS := -std=c11 -I $(INCLUDE_DIR)
CFLAGS_DEBUG := $(CFLAGS) -Wpedantic -Wall -Wextra \
					-g -fsanitize=address,pointer-compare,pointer-subtract,leak,undefined,null,bounds,alignment

INSTALL_PATH := /usr/local/bin/

ARCH := $(shell uname -m)
export CC

.PHONY: all clean install debug SET_CC
.SILENT: $(TARGET_DIR)

all: $(TARGET_DIR) $(TARGET)

$(TARGET_DIR):
	mkdir $@

$(TARGET): $(SRC_DIR)/*
	$(SET_CC)
	$(CC) $^ $(CFLAGS) -O2 -o $(TARGET_DIR)/$@

install:
	$(CP) $(TARGET_DIR)/$(TARGET) $(INSTALL_PATH)

clean:
	$(RM) -r $(TARGET_DIR)

SET_CC:
ifdef CC
    $(info CC is set, using for compiling $(CC))
else
    $(info WARNING: CC is not set)
    ifneq ($(ARCH),aarch64)
        ifneq ($(shell command -v aarch64-linux-gnu-gcc),)
            $(info Defaulting to aarch64-linux-gnu-gcc)
            $(eval CC := $(shell command -v aarch64-linux-gnu-gcc))
        else
            $(error aarch64-linux-gnu-gcc not found)
            $(error Install aarch64-linux-gnu-gcc or set CC to aarch64 compiler)
        endif
    else
        ifneq ($(shell command -v gcc),)
            $(info WARNING: Defaulting to gcc)
            $(eval CC := $(shell command -v gcc))
        else
            $(error gcc not found)
            $(error Install gcc or set CC variable to compiler of choice)
        endif
    endif
endif
