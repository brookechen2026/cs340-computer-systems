// Flip the last block and and forth in size

#include "testharness.h"

const char *mytest(allocator *a) {
  int *array = a->malloc(sizeof(int) * 100);
  int *other = a->malloc(sizeof(int) * 100);
  for(int i=0; i<20; i+=1) {
    array = a->realloc(array, sizeof(int) * 1000);
    array = a->realloc(array, sizeof(int) * 10);
  }
  return NULL;
}
