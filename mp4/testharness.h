#include <stddef.h>

typedef struct {
  void *(*malloc)(size_t size);
  void (*free)(void *ptr);
  void *(*realloc)(void *ptr, size_t size);
} allocator;
