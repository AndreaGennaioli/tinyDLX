#include "devices/dlx_timer.h"
#include "dlx_defs.h"
#include <stdlib.h>

static void d_tick(void *state);
static void d_free(void *state);
static uint32_t d_read(void *state, uint32_t offset, uint8_t bytes);
static void d_write(void *state, uint32_t offset, uint32_t data,
                    uint8_t bytes);

DLX_device *dlx_timer_create(uint32_t base_address, DLX_ic_base *ic,
                             uint8_t int_index) {
  DLX_device *dev = (DLX_device *)malloc(sizeof(DLX_device));
  if(dev == NULL) return NULL;

  dev->base_address = base_address;
  // Two words: 0..3 for writing and reading the period register
  //                 Only word writes and word reads are accepted
  //            4..7 for reading the status register
  //                 Can be a word, half-word or single byte read
  dev->range_size = 8;
  dev->state = malloc(sizeof(TimerState));

  if(dev->state == NULL) {
    free(dev);
    return NULL;
  }

  ((TimerState *)dev->state)->ic = ic;
  ((TimerState *)dev->state)->period = 0;
  ((TimerState *)dev->state)->counter = 0;
  ((TimerState *)dev->state)->status = 0;
  ((TimerState *)dev->state)->int_index = int_index;

  dev->tick = d_tick;
  dev->free = d_free;
  dev->read = d_read;
  dev->write = d_write;

  return dev;
}

static void d_tick(void *state) {
  TimerState *s = (TimerState *)state;
  if(s->period == 0) return;

  s->counter++;
  if(s->counter >= s->period) {
    s->counter = 0;
    s->status = 1;
  }

  if (s->status == 1) {
    s->ic->assert_interrupt(s->ic, s->int_index);
  }
}

static void d_free(void *state) { free(state); }

static uint32_t d_read(void *state, uint32_t offset, uint8_t bytes) {
  TimerState *s = (TimerState *)state;

  if(offset == 0 && bytes == 4) {
    return s->period;
  }

  if(  (offset == 0x4 && bytes == 4)
    || (offset == 0x6 && bytes == 2)
    || (offset == 0x7 && bytes == 1)) {
    // Dummy read to ack the asserted interrupt
    uint8_t old = s->status & 0x1;
    s->status = 0;
    s->ic->deassert_interrupt(s->ic, s->int_index);
    return old;
  }

  return 0;
}

static void d_write(void *state, uint32_t offset, uint32_t data, uint8_t bytes) {
  TimerState *s = (TimerState *)state;

  // No half-word or single byte write are allowed
  if(offset == 0 && bytes == 4) {
    // Write to the period register
    s->period = data;
    s->counter = 0;
    s->status = 0;
    s->ic->deassert_interrupt(s->ic, s->int_index);
  }
}
