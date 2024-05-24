SHELL=/bin/bash

TEST_FILE := a.S

OBJCOPY := aarch64-linux-gnu-objcopy
AS := aarch64-linux-gnu-as
MYASM := ../build/myasm

OUT_MINE := text_mine
OUT_TEST := text_test
XXD := xxd -b -c 4

.PHONY: all clean assemble_with_mine assemble_with_test test
all: test

assemble_with_mine: $(TEST_FILE)
	$(MAKE) -C .. --no-print-directory 2> /dev/null
	$(MYASM) $^ -o $@
	$(OBJCOPY) -O binary --only-section=.text $@ $(OUT_MINE)

assemble_with_test: $(TEST_FILE)
	$(AS) $^ -o $@
	$(OBJCOPY) -O binary --only-section=.text $@ $(OUT_TEST)

test: assemble_with_mine assemble_with_test 
	diff <($(XXD) $(OUT_MINE)) <($(XXD) $(OUT_TEST))
	
clean:
	$(RM) $(OUT_MINE) $(OUT_TEST) assemble_with_mine assemble_with_test test


