ESRC = $(wildcard extractuiucchunk.c)
ISRC = $(wildcard insertuiucchunk.c)

ifneq ($(ESRC),extractuiucchunk.c)
	ESRC = tests/ext.backup
endif

ifneq ($(ISRC),insertuiucchunk.c)
	ISRC = tests/ins.backup
endif

.PHONY: all, clean, test

all: extractuiucchunk insertuiucchunk

clean:
	rm -rf temp __pycache__ tests/__pycache__ .pytest_cache extractuiucchunk insertuiucchunk

test: extractuiucchunk insertuiucchunk
	python3 -m pytest

extractuiucchunk: $(ESRC)
ifeq ($(ESRC),extractuiucchunk.c)
	cc $^ -o $@
else
	rm -f $@
	ln $^ $@
endif

insertuiucchunk: $(ISRC)
ifeq ($(ISRC),insertuiucchunk.c)
	cc $^ -o $@
else
	rm -f $@
	ln $^ $@
endif
