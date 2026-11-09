#include <unistd.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include "allocator.h"
#include "testharness.h"

static void *allmem;
static int memBits;
static size_t memUsed = 0;


// global error used to note problems with allocator
static const char *error = NULL;

////////////////////////////////////////////////////////////////////
// brute force non-overlap tracker for small number of allocations
#define MAX_REGIONS 256
static struct { void *p; size_t s; } regions[MAX_REGIONS];
static int usedRegions = 0;

// track a new used region
static void trackAdd(void *p, size_t s, int resize) {
  if (p < allmem) { error = "Allocated illegal address"; return; }
  if (p+s >= allmem+(1uL<<memBits)) { error = "Allocation overflowed"; return; }
  
  // check all used regions to see if this overlaps any of them
  int dest = -1;
  for(int i=0; i<usedRegions; i+=1) {
    if (resize && regions[i].p == p) { dest = i; continue; }
    if (regions[i].p+regions[i].s > p && p+s > regions[i].p) { 
      error = "Allocated already-used memory";
      return;
    }
    if (dest < 0 && regions[i].s == 0) dest = i; // found an unused block
  }
  // new region, no unused to replace; put it at end, or replace a track entry
  if (dest < 0) {
    if (usedRegions < MAX_REGIONS) { dest = usedRegions; usedRegions += 1; }
    else dest = (((size_t)p)*91) % MAX_REGIONS; // replace randomish block
  }
  regions[dest].p = p;
  regions[dest].s = s;
  size_t newUse = p+s-allmem;
  
  // track total memory usage
  if (newUse > memUsed) memUsed = newUse;
}
// stop tracking after free
static void trackFree(void *p) {
  for(int i=0; i<usedRegions; i+=1) {
    if (regions[i].p == p) {
      regions[i].s = 0;
      regions[i].p = 0;
      if (i == usedRegions-1) usedRegions -= 1;
    }
  }
}
////////////////////////////////////////////////////////////////////


// track malloc, both memory use and correctness
void *wrapmalloc(size_t size) {
  if (error) return NULL;
  void *ans = mymalloc(size);
  trackAdd(ans, size, 0);
  if (error) return NULL;
  return ans;
}
// track free
void wrapfree(void *ptr) {
  if (error) return;
  myfree(ptr);
  trackFree(ptr);
}
// track remalloc, both memory use and correctness
void *wraprealloc(void *ptr, size_t size) {
  if (error) return NULL;
  void *ans = myrealloc(ptr, size);
  if (ans == ptr) {
    trackAdd(ans, size, 1);
  } else {
    trackAdd(ans, size, 0);
    trackFree(ptr);
  }
  if (error) return NULL;
  return ans;
}

// track malloc, just memory use (faster)
void *wrapmalloc2(size_t size) {
  void *ans = mymalloc(size);
  size_t newUse = ans+size-allmem;
  if (newUse > memUsed) memUsed = newUse;
  return ans;
}
// track remalloc, just memory (faster)
void *wraprealloc2(void *ptr, size_t size) {
  if (error) return NULL;
  void *ans = myrealloc(ptr, size);
  size_t newUse = ans+size-allmem;
  if (newUse > memUsed) memUsed = newUse;
  return ans;
}

// reset between tests
static void resetTracing() {
  memUsed = 0;
  usedRegions = 0;
  error = NULL;

  static int blank = 0x00;
  blank += 97;
  memset(allmem, blank, 1<<memBits);
}


static int unwrap_mode = 0;

static allocator safe_alloc = {wrapmalloc, wrapfree, wraprealloc};
static allocator fast_alloc = {wrapmalloc2, myfree, wraprealloc2};


// prep to catch sigsegv (segfault)
static sigjmp_buf jump_point;
static void ignore_signal(int sig, siginfo_t *unused1, void *unused2) {
  longjmp(jump_point, 1);
}


struct testresult { int memuse; unsigned long long nsec; };

// run a test case. sofilename *must* include a '/' (example: "./mytest.so" not "mytest.so")
static struct testresult runTest(const char *sofilename) {
  struct sigaction sa;
  struct testresult ans = {-1, 0};

  // catch sigsegv so that if this test crashes the program keeps running
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_flags = SA_NODEFER;
  sa.sa_sigaction = ignore_signal;
  sigaction(SIGSEGV, &sa, NULL);

  if (setjmp(jump_point) == 0) { // normal call
    // open the library in question
    void *dll = dlopen(sofilename, RTLD_NOW);
    const char *(*test)(allocator *) = dlsym(dll, "mytest");

    // run 5 times, keeping best timing result
    struct timespec t0, t1;
    unsigned long long bestnsec = 0xFFFFFFFFFFFFFFFFuL;
    for(int i=0; i<5; i+=1) { // run 5 times, keep best timing result
      allocator_reset();
      resetTracing();
      clock_gettime(CLOCK_MONOTONIC, &t0);
      const char *ans = test(i ? &fast_alloc : &safe_alloc); // only use safe_alloc on first run
      if (!error) error = ans;
      if (error) break;
      clock_gettime(CLOCK_MONOTONIC, &t1);
      unsigned long long nsec = (t1.tv_sec*1000000000uL + t1.tv_nsec) - (t0.tv_sec*1000000000uL + t0.tv_nsec);
      if (nsec < bestnsec) bestnsec = nsec;
    }
    if (error) {
      printf("❌ %-32s %s\n", sofilename, error);
      error = NULL;
    } else {
      ans.memuse = (int)memUsed;
      ans.nsec = bestnsec;
      printf("✅ %-32s %12d B  %12llu ns\n", sofilename, ans.memuse, ans.nsec);
    }
    
    dlclose(dll); // clean up

  } else { // longjump call, only happens if sigsegv happens during normal call
    if (!error) error = "Segfault generated";
    printf("❌ %-30s %s\n", sofilename, error);
  }
  
  // resume normal sigsegv handling
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = SIG_DFL;
  sigaction(SIGSEGV, &sa, NULL);
  
  return ans;
}

// helper to identify file name extensions
static int endsWith(const char *haystack, const char *needle) {
  const char *h2 = haystack + strlen(haystack) - strlen(needle);
  return !strcmp(h2, needle);
}

// specific cases mentioned in the MP description
static struct { const char *so; int isTime; long long limit; int step; } key[] = {
  { "workloads/bst_simple.so", 2, 240000, 1}, // 2 means "must be *more* space than"
  { "workloads/realloc_jitter.so", 0, 10000, 2},
  { "workloads/small_resize.so", 0, 2000, 3 },
  { "workloads/backstep.so", 0, 300000, 4 }, // breaks sometime?
  { "workloads/mergesort_like.so", 0, 300000, 4 },
  { "workloads/chaos_reuse.so", 0, 2000, 5 }, // breaks with 7
  { "workloads/chaos_reuse_2.so", 0, 20000, 5 }, // breaks with 7
  { "workloads/linked_list.so", 1, 10000000, 6 },
  { "workloads/small_resize_2.so", 0, 4000, 7 },
  { "workloads/chaos_reuse.so", 0, 2000, 8 },
  { "workloads/chaos_reuse_2.so", 0, 20000, 8 },
  { "workloads/fragmented_growing.so", 1, 20000000, 9 },
  { "workloads/fragmented_growing.so", 0, 5000000, 9 },
  { NULL, 0, 0, 0 }
};

static const char *step_names[] = {
  "",
  "metadata",
  "shrink efficiently",
  "grow at end of heap",
  "shrink at end of heap",
  "reuse free memory",
  "free list",
  "block merging",
  "block splitting",
  "size-aware free list",
};

// usage: ./tester -- runs full suite
// usage: ./tester ./mytest.so -- runs just that one test
// usage: ./tester 29 -- runs full test suite with 2^29 bytes of memory (512 MiB)
// usage: ./tester 23 ./mytest.so -- runs just one test with 2^23 bytes of memory (8 MiB)
int main(int argc, char *argv[]) {
  memBits = 0;
  if (argc >= 2) memBits = atoi(argv[1]);
  if (memBits != 0) { argv += 1; argc -= 1; }
  if (memBits < 3 || memBits > 31) {
    fprintf(stderr, "No memory size given, defaulting to 128MiB\n");
    memBits = 27;
  }
  int err;
  if ((err = posix_memalign(&allmem, 1uL<<memBits, 1uL<<memBits))) {
    fprintf(stderr, "ERROR: not enough hardware memory to allocate 2^%d bytes to this test\n", memBits);
    return err;
  }
  

  allocator_init(allmem);

  if (argc < 2) {
    runTest("./mytest.so");
    DIR *workloads = opendir("workloads");
    char soname[512];
    struct dirent *so_entry;
    int num_failed = 0;
    int steps_passed = ~0;
    while((so_entry = readdir(workloads)) != NULL) {
      if (endsWith(so_entry->d_name, ".so")) {
        strncpy(soname, "workloads/", 512);
        strncat(soname, so_entry->d_name, 500);
        struct testresult res = runTest(soname);
        if (res.memuse < 0) num_failed += 1;
        for (int i=0; key[i].so; i+=1) {
          if (!strcmp(key[i].so, soname)) {
            if (key[i].isTime == 2) {
              if (res.memuse <= key[i].limit) steps_passed &= ~(1<<key[i].step);
            } else if (key[i].isTime) {
              if (res.memuse < 0 || res.nsec > key[i].limit) steps_passed &= ~(1<<key[i].step);
            } else {
              if (res.memuse < 0 || res.memuse > key[i].limit) steps_passed &= ~(1<<key[i].step);
            }
          }
        }
      }
    }
    closedir(workloads);
    int score = 0;
    if (num_failed == 0) {
      score += 10;
      printf("✅ functional allocator\n");
      for(int step = 1; step <= 6; step += 1) {
        if (steps_passed & (1<<step) && (step == 6? steps_passed & (1<<5) : 1)) {
          printf("✅ step %d: %s\n", step, step_names[step]);
          score += step*2+1;
          if (steps_passed & ~((-1)<<step)) score += step+1;
        } else {
          printf("❌ step %d: %s\n", step, step_names[step]);
        }
      }
      if ((3<<7) == (steps_passed & (3<<7))) {
        printf("✅ steps 7 and 8: %s and %s\n", step_names[7], step_names[8]);
        score += 15;
      } else {
        printf("❌ steps 7 and 8: %s and %s\n", step_names[7], step_names[8]);
      }


      if (score == 100) {
        puts("🙌 perfect score; checking for extra credit");
        if (steps_passed & (1<<9)) {
          printf("✅ step 9: %s\n", step_names[9]);
          score += 10;
        } else {
          printf("❌ step 9: %s\n", step_names[9]);
        }
      }
      
      printf("SCORE: %d / 100\n", score);
    } else {
      printf("❌ functional allocator (incorrect results on %d workloads)\n", num_failed);
    }
  } else {
    for(int i=1; i<argc; i+=1) {
      runTest(argv[i]);
    }
  }
  

  return 0;
}
