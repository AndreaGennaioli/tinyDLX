#ifndef DLX_INPUT_PORT_H
#define DLX_INPUT_PORT_H

#include "dlx_defs.h"

typedef struct {
  DLX_ic_base *ic;
  uint8_t data;
  uint8_t ready;
  uint8_t int_index;
} InputPortState;

DLX_device *dlx_input_port_create(uint32_t base_address, DLX_ic_base *ic,
                                  uint8_t int_index);

#endif // !DLX_INPUT_PORT_H
