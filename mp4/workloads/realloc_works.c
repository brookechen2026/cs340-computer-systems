// Checking that data is copied during realloc

#include "testharness.h"

const char *mytest(allocator *a) {
  for(size_t basesize = 0x10000; basesize < 0x100000000; basesize <<= 1) {
    int *array1 = a->malloc(sizeof(int) * basesize);
    int *array2 = a->malloc(sizeof(int) * basesize);
    int *array3 = a->malloc(sizeof(int) * basesize);
    int *array4 = a->malloc(sizeof(int) * basesize);
    for(int i=0; i<basesize; i+=1) {
      array2[i] = 0x61*i;
    }
    int *array5 = a->realloc(array2, sizeof(int) * basesize*2);
    if (array2 == array5) {
      a->free(array1);
      a->free(array3);
      a->free(array4);
      a->free(array5);
      continue; // not large enough to have moved
    }
    for(int i=0; i<basesize; i+=1) {
      if (array5[i] != 0x61*i) return "realloc didn't copy data to new region";
    }
    a->free(array1);
    a->free(array3);
    a->free(array4);
    a->free(array5);
    return 0;
  }
  return "realloc never moved to new memory";
}
