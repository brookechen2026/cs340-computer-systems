#include "utf8lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <signal.h>

size_t testsRun = 0;
size_t testsPassed = 0;

int tester(int passed, const char *msg) {
  testsRun += 1;
  if (passed) {
    testsPassed += 1;
    printf("✅ %s\n", msg);
  } else {
    printf("❌ %s\n", msg);
  }
  return passed;
}
int tester2(int passed, const char *msg, int weight) {
  testsRun += weight;
  if (passed) {
    testsPassed += weight;
    printf("✅ %s (%dpt)\n", msg, weight);
  } else {
    printf("❌ %s (%dpt)\n", msg, weight);
  }
  return passed;
}

void interrupt_handler(int signo) {
  tester2(0, "timed out; skipping all remaining tests", 100-testsRun);
  printf("SCORE: %zu / %zu\n", testsPassed, testsRun);
  exit(0);
}
void segfault_handler(int signo) {
  tester2(0, "segfault running next test case; skipping all remaining tests", 100-testsRun);
  printf("SCORE: %zu / %zu\n", testsPassed, testsRun);
  exit(0);
}

void allTests() {
  signal(SIGINT, interrupt_handler);
  signal(SIGSEGV, segfault_handler);

  const char *onebytec = "\x10\x20\x37\x75";
  const int onebytei[] = { 0x10, 0x20, 0x37, 0x75, 0};
  const char *twobytec = "\xd3\xa8\xd9\x96\xc8\x94\xd4\xb7";
  const int twobytei[] = { 0x4e8, 0x656, 0x214, 0x537, 0};
  const char *threebytec = "\xe1\xa5\x84\xe1\xa8\x8d\xe4\x8d\x8e\xea\x88\xa2";
  const int threebytei[] = { 0x1944, 0x1a0d, 0x434e, 0xa222, 0};
  const char *fourbytec = "\xf4\x8d\x80\x8a\xf3\xb8\xa8\xb5\xf0\x99\x9b\xa0\xf0\xa0\x86\x83";
  const int fourbytei[] = { 0x10d00a, 0xf8a35, 0x196e0, 0x20183, 0};
  
  const char *cp;
  cp = onebytec;
  tester(decodeCharacter(&cp) == onebytei[0], "decode 1-byte");
  tester(cp == onebytec+1, "advance 1 after decode");
  tester(decodeCharacter(&cp) == onebytei[1] && decodeCharacter(&cp) == onebytei[2] && decodeCharacter(&cp) == onebytei[3] && decodeCharacter(&cp) == onebytei[4], "decode several 1-byte");

  cp = twobytec;
  tester(decodeCharacter(&cp) == twobytei[0], "decode 2-byte");
  tester(cp == twobytec+2, "advance 2 after decode");
  tester(decodeCharacter(&cp) == twobytei[1] && decodeCharacter(&cp) == twobytei[2] && decodeCharacter(&cp) == twobytei[3] && decodeCharacter(&cp) == twobytei[4], "decode several 2-byte");

  cp = threebytec;
  tester(decodeCharacter(&cp) == threebytei[0], "decode 3-byte");
  tester(cp == threebytec+3, "advance 3 after decode");
  tester(decodeCharacter(&cp) == threebytei[1] && decodeCharacter(&cp) == threebytei[2] && decodeCharacter(&cp) == threebytei[3] && decodeCharacter(&cp) == threebytei[4], "decode several 3-byte");
  
  cp = fourbytec;
  tester(decodeCharacter(&cp) == fourbytei[0], "decode 4-byte");
  tester(cp == fourbytec+4, "advance 4 after decode");
  tester(decodeCharacter(&cp) == fourbytei[1] && decodeCharacter(&cp) == fourbytei[2] && decodeCharacter(&cp) == fourbytei[3] && decodeCharacter(&cp) == fourbytei[4], "decode several 4-byte");
  
  const char *errorString;
  
  cp = errorString = "\xB0\x87";
  tester(decodeCharacter(&cp) < 0, "detect error: start with continuation byte");
  tester(cp == errorString, "don't advance on an error");
  cp = errorString = "\xd3\x7f";
  tester(decodeCharacter(&cp) < 0 || cp != errorString, "detect error: missing continuation byte 2B[1] (too small)");
  cp = errorString = "\xd3\xd3";
  tester(decodeCharacter(&cp) < 0 || cp != errorString, "detect error: missing continuation byte 2B[1] (too big)");
  cp = errorString = "\xe1\x55\x84";
  tester(decodeCharacter(&cp) < 0 || cp != errorString, "detect error: missing continuation byte 3B[1]");
  cp = errorString = "\xe1\xa5\xa4";
  tester(decodeCharacter(&cp) < 0 || cp != errorString, "detect error: missing continuation byte 3B[2]");
  cp = errorString = "\xf4\x8d\x80";
  tester(decodeCharacter(&cp) < 0 || cp != errorString, "detect error: missing continuation byte 4B[3]");
  cp = errorString = "\xf4\x8d\xf0\x8a";
  tester(decodeCharacter(&cp) < 0 || cp != errorString, "detect error: missing continuation byte 4B[2]");
  cp = errorString = "\xf4\x7d\x80\x8a";
  tester(decodeCharacter(&cp) < 0 || cp != errorString, "detect error: missing continuation byte 4B[1]");

  cp = errorString = "\xc1\x85";
  tester(decodeCharacter(&cp) < 0 || cp != errorString, "detect error: 1-byte character encoded in 2 bytes");
  cp = errorString = "\xe0\x81\xa3";
  tester(decodeCharacter(&cp) < 0 || cp != errorString, "detect error: 1-byte character encoded in 3 bytes");
  cp = errorString = "\xf0\x80\x80\x80";
  tester(decodeCharacter(&cp) < 0 || cp != errorString, "detect error: 1-byte character encoded in 4 bytes");
  cp = errorString = "\xe0\x94\x93";
  tester(decodeCharacter(&cp) < 0 || cp != errorString, "detect error: 2-byte character encoded in 3 bytes");
  cp = errorString = "\xf0\x80\x84\xb6";
  tester(decodeCharacter(&cp) < 0 || cp != errorString, "detect error: 2-byte character encoded in 4 bytes");
  cp = errorString = "\xf0\x8f\xbf\xbf";
  tester(decodeCharacter(&cp) < 0 || cp != errorString, "detect error: 3-byte character encoded in 4 bytes");

  
  const char *mixedString = "👁 ᴅᴏ 🧡 CSŔ!";
  tester(decodeCharacter(&mixedString) == 0x1F441, "decode full string: 👁");
  tester(decodeCharacter(&mixedString) == ' ', "decode full string: space");
  tester(decodeCharacter(&mixedString) == 0x1D05, "decode full string: ᴅ");
  tester(decodeCharacter(&mixedString) == 0x1D0F, "decode full string: ᴏ");
  tester(decodeCharacter(&mixedString) == ' ', "decode full string: space");
  tester(decodeCharacter(&mixedString) == 129505, "decode full string: 🧡");
  tester(decodeCharacter(&mixedString) == ' ', "decode full string: space");
  tester(decodeCharacter(&mixedString) == 'C', "decode full string: C");
  tester(decodeCharacter(&mixedString) == 'S', "decode full string: S");
  tester(decodeCharacter(&mixedString) == 340, "decode full string: code point 340");
  tester(decodeCharacter(&mixedString) == '!', "decode full string: !");
  tester(decodeCharacter(&mixedString) == '\0', "decode full string: NUL");
  
  tester(strlen8c("a simple test") == 13, "strlen8c with ASCII");
  tester(strlen8c("👁 ᴅᴏ 🧡 CSŔ!") == 11, "strlen8c with Unicode");
  tester(strlen8c("👁 ᴅᴏ 🧡 C\xF8SŔ!") == 8, "strlen8c with errors");


  char dest[256];
  char *dptr = dest;
  size_t space = 256;
  encodeCharacter(&dptr, &space, '1');
  tester(space == 255 && dest[0] == '1', "encode ASCII");
  encodeCharacter(&dptr, &space, 340);
  tester(space == 253 && dptr[-2] == "Ŕ"[0] && dptr[-1] == "Ŕ"[1], "encode 2-byte");
  encodeCharacter(&dptr, &space, U'ℕ');
  tester(space == 250 && dptr[-3] == "ℕ"[0] && dptr[-2] == "ℕ"[1] && dptr[-1] == "ℕ"[2], "encode 3-byte");
  encodeCharacter(&dptr, &space, U'🧶');
  tester(space == 246 && dptr[-4] == "🧶"[0] && dptr[-3] == "🧶"[1] && dptr[-2] == "🧶"[2] && dptr[-1] == "🧶"[3], "encode 4-byte");
  dptr = dest;
  space = 0;
  tester(encodeCharacter(&dptr, &space, 34) == 0 && dptr == dest && space == 0, "encoding error: not enough space 1/0");
  space = 1;
  tester(encodeCharacter(&dptr, &space, 34) == 1 && dptr == dest+1 && space == 0, "encoding with just enough space 1/1");
  dptr = dest;
  space = 0;
  tester(encodeCharacter(&dptr, &space, 340) == 0 && dptr == dest && space == 0, "encoding error: not enough space 2/0");
  space = 1;
  tester(encodeCharacter(&dptr, &space, 418) == 0 && dptr == dest && space == 1, "encoding error: not enough space 2/1");
  space = 2;
  tester(encodeCharacter(&dptr, &space, 2024) == 2 && dptr == dest+2 && space == 0, "encoding with just enough space 2/2");
  dptr = dest;
  space = 0;
  tester(encodeCharacter(&dptr, &space, 0xFFFF) == 0 && dptr == dest && space == 0, "encoding error: not enough space 3/0");
  space = 1;
  tester(encodeCharacter(&dptr, &space, 0x2024) == 0 && dptr == dest && space == 1, "encoding error: not enough space 3/1");
  space = 2;
  tester(encodeCharacter(&dptr, &space, 0x1000) == 0 && dptr == dest && space == 2, "encoding error: not enough space 3/2");
  space = 3;
  tester(encodeCharacter(&dptr, &space, 0x900) == 3 && dptr == dest+3 && space == 0, "encoding with just enough space 3/3");
  dptr = dest;
  space = 0;
  tester(encodeCharacter(&dptr, &space, 0x98765) == 0 && dptr == dest && space == 0, "encoding error: not enough space 4/0");
  space = 1;
  tester(encodeCharacter(&dptr, &space, 0x80286) == 0 && dptr == dest && space == 1, "encoding error: not enough space 4/1");
  space = 2;
  tester(encodeCharacter(&dptr, &space, 0x101010) == 0 && dptr == dest && space == 2, "encoding error: not enough space 4/2");
  space = 3;
  tester(encodeCharacter(&dptr, &space, 0x10000) == 0 && dptr == dest && space == 3, "encoding error: not enough space 4/3");
  space = 4;
  tester(encodeCharacter(&dptr, &space, 0x12345) == 4 && dptr == dest+4 && space == 0, "encoding with just enough space 4/4");
  dptr = dest;
  space = 256;
  tester(encodeCharacter(&dptr, &space, -1) == 0 && dptr == dest && space == 256, "encoding error: negative code point");
  tester(encodeCharacter(&dptr, &space, 0x110000) == 0 && dptr == dest && space == 256, "encoding error: too-large code point");
  
  tester(strlen8i(onebytei) == 4, "strlen8i of 1-byte characters");
  tester(strlen8i(twobytei) == 8, "strlen8i of 2-byte characters");
  tester(strlen8i(threebytei) == 12, "strlen8i of 3-byte characters");
  tester(strlen8i(fourbytei) == 16, "strlen8i of 4-byte characters");
  int mixed[] = {0x10ffff,0x1f,0x900, 0x80, 0x00, 0x54, 0};
  tester(strlen8i(mixed) == 10, "strlen8i of mixed characters");
  mixed[2] = 0x112233;
  tester(strlen8i(mixed) == 5, "strlen8i with invalid code point");
  tester(strlen8i(onebytei+4) == 0, "strlen8i of empty list");

  ReplacementSet *empty = newReplacementSet();
  ReplacementSet *noOverlap = newReplacementSet();
  ReplacementSet *withOverlap = newReplacementSet();
  
  addReplacement(noOverlap, "smile", "☺");
  addReplacement(noOverlap, "☺", "smile");
  addReplacement(noOverlap, "CS 340", "🧡🧡🧡");
  
  addReplacement(withOverlap, "340", "☺");
  addReplacement(withOverlap, "34", "㉞");
  addReplacement(withOverlap, "440", "🤖");
  addReplacement(withOverlap, "4000", "𐄥");
  addReplacement(withOverlap, "0004", "four");
  addReplacement(withOverlap, "CS 340", "🎉");
  addReplacement(withOverlap, "cs", "CS");
  addReplacement(withOverlap, "zero", "0");

  char *tmp, *tmp2;

  tmp = replaceAll(noOverlap, "");
  tester(!strcmp(tmp, ""), "empty string replacement");
  free(tmp);

  tmp = replaceAll(empty, "This is a CS 340 test ☺");
  tester(!strcmp(tmp, "This is a CS 340 test ☺"), "empty replacement set changes nothing");
  free(tmp);

  tmp = replaceAll(noOverlap, "CS 340");
  tester(!strcmp(tmp, "🧡🧡🧡"), "one replacement");
  free(tmp);

  tmp = replaceAll(noOverlap, "🧡🧡🧡");
  tester(!strcmp(tmp, "🧡🧡🧡"), "one-way replacements only");
  free(tmp);

  tmp = replaceAll(noOverlap, "smile");
  tester(!strcmp(tmp, "☺"), "shrinking replacment without loop");
  free(tmp);

  tmp = replaceAll(noOverlap, "☺");
  tester(!strcmp(tmp, "smile"), "growing replacment without loop");
  free(tmp);

  tmp = replaceAll(noOverlap, "-CS 340");
  tester(!strcmp(tmp, "-🧡🧡🧡"), "replacement not first");
  free(tmp);
  
  tmp = replaceAll(noOverlap, "CS 340-");
  tester(!strcmp(tmp, "🧡🧡🧡-"), "replacement not last");
  free(tmp);

  tmp = replaceAll(noOverlap, "smilesmile smile  smile smilesmile");
  tester(!strcmp(tmp, "☺☺ ☺  ☺ ☺☺"), "same replacement over and over");
  free(tmp);

  tmp = replaceAll(noOverlap, "This is a CS 340 test ☺");
  tester(!strcmp(tmp, "This is a 🧡🧡🧡 test smile"), "replace ASCII with UTF-8 and vice-versa");
  free(tmp);

  tmp = replaceAll(withOverlap, "cs 340");
  tester(!strcmp(tmp, "CS ☺"), "Replacements do not trigger later replacements");
  free(tmp);

  tmp = replaceAll(withOverlap, "cs 44zero");
  tester(!strcmp(tmp, "CS 440"), "Replacements do not trigger earlier replacements");
  tmp2 = replaceAll(withOverlap, tmp);
  tester(!strcmp(tmp2, "CS 🤖"), "Replacements can be applied repeatedly");
  free(tmp2);
  free(tmp);

  tmp = replaceAll(withOverlap, "44000");
  tester(!strcmp(tmp, "🤖00"), "First match first");
  free(tmp);

  tmp = replaceAll(withOverlap, "340");
  tester(!strcmp(tmp, "☺"), "Longest match first");
  tmp2 = replaceAll(noOverlap, tmp);
  tester(!strcmp(tmp2, "smile"), "Mix replacement sets");
  free(tmp2);
  free(tmp);

  tmp = replaceAll(withOverlap, "34000");
  tester(!strcmp(tmp, "☺00"), "First longest match first");
  free(tmp);

  tmp = replaceAll(withOverlap, "3400004");
  tester(!strcmp(tmp, "☺four"), "match unlocked by earlier match");
  free(tmp);

  tmp = replaceAll(withOverlap, "3434034003400034000040000CS340CS 34OCS 3404000");
  tester(!strcmp(tmp, "㉞☺☺0☺00☺four0000CS☺CS ㉞O🎉𐄥"), "various overlapping replacements");
  free(tmp);

  addReplacement(withOverlap, "340340", "䷡");
  tmp = replaceAll(withOverlap, "3434034003400034000040000CS340CS 34OCS 3404000");
  tester(!strcmp(tmp, "㉞䷡0☺00☺four0000CS☺CS ㉞O🎉𐄥"), "modify replacement set after use");
  free(tmp);

  
  tmp = replaceAll(empty, "3434034003400034000040000CS340CS 34OCS 340");
  tester(!strcmp(tmp, "3434034003400034000040000CS340CS 34OCS 340"), "empty replacement set changes nothing after other replacements");
  free(tmp);

  char *huge = malloc(1000001);
  for(int i=0; i<1000000; i+=1) huge[i] = '3'+(i&1);
  huge[1000000] = 0;
  char *huge2 = replaceAll(withOverlap, huge);

  tester2(strlen(huge2) == 1500000, "one million 3-byte characters", 7);
  tester(huge2[3000] == "㉞"[0], "one million 3-byte characters, random one starts correctly");
  tester(huge2[60001] == "㉞"[1], "one million 3-byte characters, random one correct in middle");
  tester(huge2[902] == "㉞"[2], "one million 3-byte characters, random one ends correctly");
  free(huge2);
  free(huge);
    
  deleteReplacementSet(empty);
  deleteReplacementSet(noOverlap);
  deleteReplacementSet(withOverlap);

  printf("SCORE: %zu / %zu\n", testsPassed, testsRun);
}

int main() {
  pid_t child;
  if ((child = fork()) == 0) { // one process to run the tests
    allTests();
  } else { // and another to keep the first from running too long
    int status;
    int got = 1;
    for(int i=0; i<20; i+=1) { // 20*0.1s = 2 seconds
      usleep(100000); // 0.1 seconds
      got = waitpid(child, &status, WNOHANG);
      if (got == child) {
        if (WIFSIGNALED(status)) {
          printf("Code crashed via a %s\n", strsignal(WTERMSIG(status)));
        }
        break;
      }
    }
    
    if (got == 0) kill(child, SIGINT);
  }
  return 0;
}
