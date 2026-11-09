// A naive ArrayList/vector implementation might grow an array one int at a time

#include "testharness.h"

const char *mytest(allocator *a) {
  int *array = a->malloc(sizeof(int));
  array[0] = 0;
  for(int i=2; i<=100; i+=1) {
    array = a->realloc(array, sizeof(int)*i);
    array[i-1] = i;
  }
  a->free(array);
  return 0;
}
