CXX = c++
CC = $(CXX) -x c -g -Wall 
CFLAGS_CATCH = -fpermissive -w $(CS340) -pthread -g -std=c++11
CFLAGS = -W -Wall $(CS340) -pthread

all: staticserver tester

staticserver: http.o server.o
	$(CXX) $(CFLAGS) $^ -o $@

http.o: http.c
	$(CC) $(CFLAGS) $^ -c -o $@

server.o: server.c
	$(CC) $(CFLAGS) $^ -c -o $@

## Test Suite
tests/test.o: tests/test.cpp 
	$(CXX) $(CFLAGS_CATCH) $^ -c -o $@

tests/test-part1.o: tests/test-part1.cpp
	$(CXX) $(CFLAGS_CATCH) $^ -c -o $@

tests/test-part2.o: tests/test-part2.cpp
	$(CXX) $(CFLAGS_CATCH) $^ -c -o $@

tester: tests/test.o tests/test-part1.o tests/test-part2.o http.o 
	$(CXX) $(CFLAGS_CATCH) $^ -o $@

test: tester staticserver
	bash tests.sh

.PHONY: all clean test
clean:
	rm -rf *.o tests/*.o staticserver tester TEST_out* __pycache__ .pytest_cache
