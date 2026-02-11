#ifndef DLX_MEMORY_BUS_H
#define DLX_MEMORY_BUS_H

#include "dlx_defs.h"
#include <stdint.h>

uint32_t dlx_memory_read_word(DLX_state *state, uint32_t address,
                              uint8_t convert_endianess);

#endif // !DLX_MEMORY_BUS_H
