// randomly scattered free blocks with mostly increasing sizes

#include "testharness.h"

const char *mytest(allocator *a) {
  for(int i=0; i<0xFFF; i+=1) {
    for(int k = 0; k < 8; k += 1) {
      void *tmp = a->malloc(1 + (i&~(1<<k)));
      void *leak = a->malloc(1);
      a->free(tmp);
    }
  }
  
  return 0;
}
