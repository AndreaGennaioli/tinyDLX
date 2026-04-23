#include "devices/dlx_startup_circuit.h"
#include <stdlib.h>

static void d_free(void *state);
static uint32_t d_read(void *state, uint32_t offset, uint8_t bytes);
static void d_write(void *state, uint32_t offset, uint32_t data, uint8_t bytes);

DLX_device *dlx_startup_circuit_create(uint32_t base_address) {
  DLX_device *dev = (DLX_device *)malloc(sizeof(DLX_device));
  if(dev == NULL) return NULL;

  dev->base_address = base_address;
  dev->range_size = 1;
  dev->state = malloc(sizeof(uint8_t));
  
  if(dev->state == NULL) {
    free(dev);
    return NULL;
  }

  *(uint8_t *)dev->state = 1;

  dev->tick = NULL;
  dev->free = d_free;
  dev->read = d_read;
  dev->write = d_write;

  return dev;
}

static void d_free(void *state) { free(state); }

static uint32_t d_read(void *state, uint32_t offset, uint8_t bytes) {
  return 0x00000001 & *(uint8_t *)state;
}

static void d_write(void *state, uint32_t offset, uint32_t data,
                    uint8_t bytes) {
  *(uint8_t *)state = 0;
}
