#ifndef DLX_TIMER_H
#define DLX_TIMER_H

#include "dlx_defs.h"
#include <stdint.h>

typedef struct {
  DLX_ic_base *ic;
  // The period register, if 0 the timer is disabled
  uint32_t period;
  // The clock counter.
  uint32_t counter;
  // Status register: 0 = idle, 1 = interrupt asserted
  // Reading this register returns its value, clears it
  // and deasserts the interrupt (ack).
  // The timer interrupt is level-triggered until ack.
  uint8_t status;
  uint8_t int_index;
} TimerState;

DLX_device *dlx_timer_create(uint32_t base_address, DLX_ic_base *ic,
                             uint8_t int_index);

#endif // !DLX_TIMER_H
