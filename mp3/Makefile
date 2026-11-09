CFLAGS = -g -Wall -Werror


.PHONY: all clean test

all: pngchunklist extractuiucchunk insertuiucchunk

clean:
	rm -f *.o pngchunklist extractuiucchunk insertuiucchunk

test:
	python3 tests.py

extractuiucchunk: extractuiucchunk.c pnglib.o
	cc $(CFLAGS) $^ -o $@

insertuiucchunk: insertuiucchunk.c pnglib.o
	cc $(CFLAGS) $^ -o $@

pngchunklist: pngchunklist.c pnglib.o
	cc $(CFLAGS) $^ -o $@

pnglib.o: pnglib.c pnglib.h
	cc $(CFLAGS) -c $<

