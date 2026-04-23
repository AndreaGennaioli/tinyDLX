#include "devices/dlx_ic.h"
#include <stdlib.h>

static void assert_interrupt(struct DLX_ic_base *ic, uint8_t index);
static void deassert_interrupt(struct DLX_ic_base *ic, uint8_t index);
static void d_tick(void *state);
static void d_free(void *state);
static uint32_t d_read(void *state, uint32_t offset, uint8_t bytes);

DLX_device *dlx_ic_create(uint32_t base_address, uint32_t devices_size,
                          void (*controller_assert_interrupt)(void *state),
                          void *controller_state) {
  DLX_device *dev = (DLX_device *)malloc(sizeof(DLX_device));

  dev->base_address = base_address;
  dev->range_size = 1;
  dev->state = malloc(sizeof(ICState));

  ICState *ic_state = (ICState *)dev->state;
  for (int i = 0; i < DLX_MAX_DEVICES; i++) {
    ic_state->base.lines[i] = 0;
  }
  ic_state->devices_size = devices_size;
  ic_state->controller_state = controller_state;
  ic_state->base.assert_interrupt = assert_interrupt;
  ic_state->base.deassert_interrupt = deassert_interrupt;
  ic_state->base.controller_assert_interrupt = controller_assert_interrupt;

  dev->tick = d_tick;
  dev->free = d_free;
  dev->read = d_read;
  dev->write = NULL;

  return dev;
}

static void deassert_interrupt(struct DLX_ic_base *ic, uint8_t index) {
  ic->lines[index] = 0;
}

static void assert_interrupt(struct DLX_ic_base *ic, uint8_t index) {
  ic->lines[index] = 1;
}

static void d_tick(void *state) {
  ICState *ic_state = (ICState *)state;

  for (int i = 0; i < ic_state->devices_size; i++) {
    if (ic_state->base.lines[i]) {
      ic_state->base.controller_assert_interrupt(ic_state->controller_state);
      break;
    }
  }
}

static void d_free(void *state) { free(state); }

static uint32_t d_read(void *state, uint32_t offset, uint8_t bytes) {
  ICState *ic_state = (ICState *)state;
  int i;

  for (i = 0; i < ic_state->devices_size; i++) {
    if (ic_state->base.lines[i]) {
      ic_state->base.lines[i] = 0;

      return i;
    }
  }

  return 0xFF;
}
