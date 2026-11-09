// Several naive ArrayList/vector implementations concurrently

#include "testharness.h"

const char *mytest(allocator *a) {
  int *array1 = a->malloc(sizeof(int));
  int *array2 = a->malloc(sizeof(int));
  array1[0] = 0;
  array2[0] = 0;
  for(int i=2; i<=100; i+=1) {
    if (i%3) {
      array1 = a->realloc(array1, sizeof(int)*i);
      array1[i-1] = i;
    } else {
      array2 = a->realloc(array2, sizeof(int)*i);
      array2[i-1] = i;
    }
  }
  a->free(array1);
  a->free(array2);
  return 0;
}
