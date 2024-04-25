SHELL=/bin/bash

TEST_FILE := test.S

OBJCOPY := aarch64-linux-gnu-objcopy
AS := aarch64-linux-gnu-as
MYASM := ../build/myasm

CC := clang

OUT_MINE := a.out
OUT_TEST := text_test

.PHONY: all clean assemble_with_mine assemble_with_test test distclean
all: test

.SILENT: assemble_with_mine assemble_with_test

assemble_with_mine: $(TEST_FILE)
	$(MYASM) $^

assemble_with_test: $(TEST_FILE)
	$(AS) $^ -o assembled_test
	$(OBJCOPY) -O binary --only-section=.text assembled_test $(OUT_TEST)

test: assemble_with_test assemble_with_mine
	diff <(xxd $(OUT_MINE)) <(xxd $(OUT_TEST))
	
clean:
	$(RM) assembled_test assembled_mine 

distclean:
	$(RM) assembled_test assembled_mine $(OUT_MINE) $(OUT_TEST)

