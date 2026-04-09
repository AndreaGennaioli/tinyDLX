#ifndef DLX_IC_H
#define DLX_IC_H

#include "dlx_defs.h"
#include <stdint.h>

typedef struct {
  DLX_ic_base base;
  uint32_t devices_size;
  void* controller_state;
} ICState;

DLX_device *dlx_ic_create(uint32_t base_address, uint32_t devices_size, void (*controller_assert_interrupt)(void *state), void* controller_state);

#endif // !DLX_IC_H
