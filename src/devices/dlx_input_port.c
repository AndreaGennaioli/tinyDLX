#include "devices/dlx_input_port.h"
#include "dlx_defs.h"
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void d_tick(void *state);
static void d_free(void *state);
static uint32_t d_read(void *state, uint32_t offset, uint8_t bytes);

DLX_device *dlx_input_port_create(uint32_t base_address, DLX_ic_base *ic,
                                  uint8_t int_index) {
  DLX_device *dev = (DLX_device *)malloc(sizeof(DLX_device));
  if(dev == NULL) return NULL;

  dev->base_address = base_address;
  dev->range_size = 1;
  dev->state = malloc(sizeof(InputPortState));

  if(dev->state == NULL) {
    free(dev);
    return NULL;
  }

  ((InputPortState *)dev->state)->ic = ic;
  ((InputPortState *)dev->state)->ready = 0;
  ((InputPortState *)dev->state)->data = 0;
  ((InputPortState *)dev->state)->int_index = int_index;

  dev->tick = d_tick;
  dev->free = d_free;
  dev->read = d_read;
  dev->write = NULL;

  return dev;
}

static void d_tick(void *state) {
  InputPortState *s = (InputPortState *)state;
  if (s->ready) {
    s->ic->assert_interrupt(s->ic, s->int_index);
    return;
  }

  struct pollfd pfd = {.fd = STDIN_FILENO, .events = POLLIN};
  if (poll(&pfd, 1, 0) > 0) {
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1) {
      s->data = c;
      s->ready = 1;
      s->ic->assert_interrupt(s->ic, s->int_index);
    }
  }
}

static void d_free(void *state) { free(state); }

static uint32_t d_read(void *state, uint32_t offset, uint8_t bytes) {
  InputPortState *s = (InputPortState *)state;
  uint8_t data = s->data;
  s->ready = 0;

  if (s->ic && s->ic->deassert_interrupt) {
      s->ic->deassert_interrupt(s->ic, s->int_index);
  }

  return (uint32_t)data;
}
