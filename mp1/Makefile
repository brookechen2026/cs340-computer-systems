CFLAGS = -g -Wall -Werror -pedantic-errors -std=c17


.PHONY: all clean test

all: dll_c dll_cpp

clean:
	rm -f dll_cpp dll_c dll.o

test:
	python3 tests.py


dll.o: dll.c dll.h
	cc $(CFLAGS) -c dll.c

dll_c: dll.o usedll.c
	cc $(CFLAGS) $^ -o $@

dll_cpp: usedll.cpp dll.hpp
	c++ $< -o $@
