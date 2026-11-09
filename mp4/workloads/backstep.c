// free what was just allocated many times

#include "testharness.h"

const char *mytest(allocator *a) {
  void **ptrs = a->malloc(sizeof(void *) * 100);
  for(int i=0; i<100; i+=1) {
    int *array = array = a->malloc(sizeof(int) * 100*(i+1));
    a->free(array);
    ptrs[i] = a->malloc(sizeof(int) * 50*(i+1));
    ptrs[i] = a->realloc(ptrs[i], 10*(i+1));
  }
  for(int i=0; i<100; i+=1) {
    a->free(ptrs[i]);
  }
  a->free(ptrs);
  return NULL;
}
