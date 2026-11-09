CFLAGS = -g -Wall -Werror -funsigned-char


.PHONY: all clean test

all: tester

clean:
	rm -f tester *.o

test: tester
	sh tests.sh

tester: utf8lib.o tester.c
	cc $(CFLAGS) $^ -o $@

utf8lib.o: utf8lib.c utf8lib.h
	cc $(CFLAGS) -c $<

