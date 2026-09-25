#include "dlx_time_utils.h"
#include <time.h>

// Returns current time in ns
uint64_t now_ns(void) {
  struct timespec ts;
  int ris = clock_gettime(CLOCK_MONOTONIC, &ts);
  return ris == 0 ? (uint64_t)ts.tv_sec * NS_PER_SEC + ts.tv_nsec : 0;
}

