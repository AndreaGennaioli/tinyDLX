#include "devices/dlx_power_manager.h"
#include "dlx_defs.h"
#include "dlx_state.h"
#include <stdlib.h>
#include <unistd.h>

static void d_free(void *state);
static void d_write(void *state, uint32_t offset, uint32_t data,
                        uint8_t bytes);

DLX_device *dlx_power_manager_create(uint32_t base_address, DLX_state *state) {
  DLX_device *dev = (DLX_device *)malloc(sizeof(DLX_device));
  if(dev == NULL) return NULL;

  dev->base_address = base_address;
  dev->range_size = 1;
  dev->state = malloc(sizeof(PowerManagerState));

  if(dev->state == NULL) {
    free(dev);
    return NULL;
  }

  ((PowerManagerState *)dev->state)->dlx_state = state;

  dev->tick = NULL;
  dev->free = d_free;
  dev->read = NULL;
  dev->write = d_write;

  return dev;
}

static void d_free(void *state) { free(state); }

static void d_write(void *state, uint32_t offset, uint32_t data,
                        uint8_t bytes) {
  PowerManagerState *s = (PowerManagerState *)state;
  dlx_exit(s->dlx_state);
}

