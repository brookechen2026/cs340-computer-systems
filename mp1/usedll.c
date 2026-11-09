#include <stdio.h>
#include "dll.h"


int main(int argc, char *argv[]) {
  dllListc cl; dllInitc(&cl);
  dllLists sl; dllInits(&sl);
  for(int i=1; i<argc; i+=1) {
    dllPushs(&sl, argv[i]);
  }
  if (dllFinds(&sl, "-h") || dllFinds(&sl, "--help") || argc < 2) {
    puts("This program is a simple linked-list demo; run it with arguments.");
    return 1;
  }
  char *value;
  while(NULL != (value = dllPops(&sl, NULL))) {
    while(*value) {
      dllEnqueuec(&cl, *value);
      ++value;
    }
    dllEnqueuec(&cl, ' ');
  }
  char c;
  while( (c = dllShiftc(&cl, '\0')) ) {
    putchar(c);
  }
  puts("");
  return 0;
}
