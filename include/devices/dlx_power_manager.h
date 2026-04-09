#ifndef DLX_POWER_MANAGER_H
#define DLX_POWER_MANAGER_H

#include "dlx_defs.h"

typedef struct {
  DLX_state *dlx_state;
} PowerManagerState;

DLX_device *dlx_power_manager_create(uint32_t base_address, DLX_state *state);

#endif // !DLX_POWER_MANAGER_H
