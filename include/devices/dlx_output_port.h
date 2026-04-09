#ifndef DLX_OUTPUT_PORT_H
#define DLX_OUTPUT_PORT_H

#include "dlx_defs.h"

typedef struct {
  uint8_t ready;
  uint8_t delay;
} OutputPortState;

DLX_device *dlx_output_port_create(uint32_t base_address);

#endif // !DLX_OUTPUT_PORT_H
