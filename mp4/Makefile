CASES := $(patsubst %.c,%.so,$(wildcard workloads/*.c))
CC := cc -Werror -g -O0 -fPIC -I.


.PHONY: all test clean build

all: tester mytest.so $(CASES)

build: tester mytest.so $(CASES)

clean:
	rm -f *.o *.so *.gch tester workloads/*.so workloads/*.o



tester: testharness.c allocator.o
	$(CC) -o $@ $^

mytest.so: mytest.o
	$(CC) -shared -fPIC $^ -o $@

%.o: %.c
	$(CC) -c $< -o $@

workloads/%.o: workloads/%.c
	$(CC) -c $< -o $@

workloads/%.so: workloads/%.o
	$(CC) -shared -fPIC $^ -o $@

	
test: tester mytest.so $(CASES)
	./tester

