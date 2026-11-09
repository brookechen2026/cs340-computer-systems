#include <string>
#include <iostream>
#include "dll.hpp"


int main(int argc, char *argv[]) {
  LinkedList::List<char> cl;
  LinkedList::List<std::string> sl;
  for(int i=1; i<argc; i+=1) {
    sl.push(std::string(argv[i]));
  }
  if (sl.find("-h") || sl.find("--help") || argc < 2) {
    std::cout << "This program is a simple linked-list demo; run it with arguments.\n";
    return 1;
  }
  std::string never = "\x01";
  std::string value;
  while(never != (value = sl.pop(never))) {
    for(auto it = value.begin(); it < value.end(); ++it) {
      cl.enqueue(*it);
    }
    cl.enqueue(' ');
  }
  char c;
  while( (c = cl.shift('\0')) ) {
    std::cout << c;
  }
  std::cout << std::endl;
  return 0;
}
