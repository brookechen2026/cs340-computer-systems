// allocate then free a huge singly linked list

#include "testharness.h"

struct lln { int data; struct lln *next; };

const char *mytest(allocator *a) {
  struct lln *head = NULL;
  for(int i=0; i<20000; i+=1) {
    struct lln *node = a->malloc(sizeof(struct lln));
    node->data = i;
    node->next = head;
    head = node;
  }
  while(head) {
    struct lln *node = head;
    head = head->next;
    a->free(node);
  }
  return 0;
}
