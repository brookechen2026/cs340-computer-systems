#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

// predeclaration of four datatypes
struct dllNodec;
struct dllNodes;
struct dllListc;
struct dllLists;

// The equivalent of Node<char> from dll.hpp
typedef struct dllNodec {
  char data;
  struct dllNodec *next;
  struct dllNodec *prev;
} dllNodec;

// The equivalent of Node<string> from dll.hpp
typedef struct dllNodes {
  char *data;
  struct dllNodes *next;
  struct dllNodes *prev;
} dllNodes;

// the equivalent of Node::detach from dll.hpp
void dllDetachc(dllNodec *self);
void dllDetachs(dllNodes *self);


// The equivalent of List<char> from dll.hpp
typedef struct dllListc {
  dllNodec *head, *tail;
} dllListc;

// The equivalent of List<string> from dll.hpp
typedef struct dllLists {
  dllNodes *head, *tail;
} dllLists;


// The equivalent of List() from dll.hpp
void dllInitc(dllListc *self);
void dllInits(dllLists *self);

// the equivalent of ~List() from dll.hpp
void dllClearc(dllListc *self);
void dllClears(dllLists *self);

// the equivalent of List::find(value) from dll.hpp
dllNodec *dllFindc(const dllListc *self, const char value);
dllNodes *dllFinds(const dllLists *self, const char *value);

// the equivalent of List::pop(ifEmpty) from dll.hpp
char dllPopc(dllListc *self, char ifEmpty);
char *dllPops(dllLists *self, char *ifEmpty);

// the equivalent of List::shift(ifEmpty) from dll.hpp
char dllShiftc(dllListc *self, char ifEmpty);
char *dllShifts(dllLists *self, char *ifEmpty);

// the equivalent of List::push(value) from dll.hpp
void dllPushc(dllListc *self, char value);
void dllPushs(dllLists *self, char *value);

// the equivalent of List::enqueue(value) from dll.hpp
void dllEnqueuec(dllListc *self, char value);
void dllEnqueues(dllLists *self, char *value);

char *my_strdup(const char *src);
#endif

