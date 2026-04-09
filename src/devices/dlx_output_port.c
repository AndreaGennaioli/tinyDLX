#include "devices/dlx_output_port.h"
#include "dlx_defs.h"
#include "debug.h"
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define OUTPUT_PORT_DELAY 3

static void d_tick(void *state);
static void d_free(void *state);
static uint32_t d_read(void *state, uint32_t offset, uint8_t bytes);
static void d_write(void *state, uint32_t offset, uint32_t data,
                        uint8_t bytes);

DLX_device *dlx_output_port_create(uint32_t base_address) {
  DLX_device *dev = (DLX_device *)malloc(sizeof(DLX_device));

  dev->base_address = base_address;
  dev->range_size = 1;
  dev->state = malloc(sizeof(OutputPortState));
  ((OutputPortState *)dev->state)->ready = 1;
  ((OutputPortState *)dev->state)->delay = 0;

  dev->tick = d_tick;
  dev->free = d_free;
  dev->read = d_read;
  dev->write = d_write;

  return dev;
}

static void d_tick(void *state) {
  OutputPortState *s = (OutputPortState *)state;
  if (s->ready)
    return;

  if (--s->delay == 0) {
    s->ready = 1;
  }
}

static void d_free(void *state) { free(state); }

static uint32_t d_read(void *state, uint32_t offset, uint8_t bytes) {
  OutputPortState *s = (OutputPortState *)state;
  return s->ready; // 1=ready, 0=busy
}

static void d_write(void *state, uint32_t offset, uint32_t data,
                        uint8_t bytes) {
  OutputPortState *s = (OutputPortState *)state;
  putchar(data & 0xFF);
  fflush(stdout);
  s->ready = 0;
  s->delay = OUTPUT_PORT_DELAY;
}
