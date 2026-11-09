// the memory allocation patterns of mergesort

#include "testharness.h"

void fake_sort(allocator *a, int *array, unsigned long size) {
  if (size <= 1) return; // sorted
  fake_sort(a, array, size/2);
  fake_sort(a, array+(size/2), size - size/2);
  int *merger = a->malloc(size*sizeof(int));
  // skipped: merging step
  // skipped: copy merger back into a
  a->free(merger);
}

const char *mytest(allocator *a) {
  unsigned long size = 12345;
  int *array = a->malloc(size*sizeof(int));
  fake_sort(a, array, size);
  return 0;
}
