#include "dlx_interrupts.h"
#include "debug.h"
#include <time.h>

static clock_t start_tick;

static void start_timer();
static void stop_timer();

void dlx_exec_debug_interrupt(uint32_t code) {
  switch (code) {
  case 0xF0:
    start_timer();
    break;
  case 0xF1:
    stop_timer();
    break;
  default:
    warn("EXECUTE: 0x%02X unknown debug interrupt code", code);
  }
}

static void start_timer() {
  start_tick = clock();
  info("Timer started");
}

static void stop_timer() {
  clock_t end_tick = clock();
  double time_diff = (double)(end_tick - start_tick) / CLOCKS_PER_SEC;
  info("Timer stopped, elapsed time: %lf seconds", time_diff);
}
