#include "dlx_interrupts.h"
#include "dlx_time_utils.h"
#include "dlx_defs.h"
#include "debug.h"
#include <errno.h>
#include <stdint.h>
#include <string.h>


static uint64_t start_time;

static void start_timer(DLX_state *state);
static void stop_timer(DLX_state *state);

void dlx_exec_debug_interrupt(uint32_t code, DLX_state *state) {
  switch (code) {
  case 0xF0:
    start_timer(state);
    break;
  case 0xF1:
    stop_timer(state);
    break;
  case 0xF2:
    state->exec_state = DLX_HALT;
    break;
  default:
    if(state->config->strict_mode) {
      error("EXECUTE: 0x%02X unknown debug interrupt code", code);
        state->exec_state = DLX_FAULT;
    } else {
      warn("EXECUTE: 0x%02X unknown debug interrupt code", code);
    }
  }
}

static void start_timer(DLX_state *state) {
  if((start_time = now_ns()) == 0) {
    error("CRITICAL: %s", strerror(errno));
    state->exec_state = DLX_CRITICAL;
  }
  info("Timer started");
}

static void stop_timer(DLX_state *state) {
  uint64_t end_time;
  if((end_time = now_ns()) == 0) {
    error("CRITICAL: %s", strerror(errno));
    state->exec_state = DLX_CRITICAL;
  }
  double time_diff = (double)(end_time - start_time) / NS_PER_SEC;
  info("Timer stopped, elapsed time: %lf seconds", time_diff);
}
