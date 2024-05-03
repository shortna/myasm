SHELL=/bin/bash

TEST_FILE := test.S

OBJCOPY := aarch64-linux-gnu-objcopy
AS := aarch64-linux-gnu-as
MYASM := ../build/myasm

OUT_MINE := text_mine
OUT_TEST := text_test

.PHONY: all clean assemble_with_mine assemble_with_test test distclean
all: test

.SILENT: assemble_with_mine assemble_with_test

assemble_with_mine: $(TEST_FILE)
	$(MYASM) $^ -o $@
	$(OBJCOPY) -O binary --only-section=.text $@ $(OUT_MINE)
	rm $@

assemble_with_test: $(TEST_FILE)
	$(AS) $^ -o $@
	$(OBJCOPY) -O binary --only-section=.text $@ $(OUT_TEST)
	rm $@

test: assemble_with_test assemble_with_mine
	diff <(xxd $(OUT_MINE)) <(xxd $(OUT_TEST))
	
clean:
	$(RM) assembled_test assembled_mine $(OUT_MINE) $(OUT_TEST)

