#ifndef DLX_MEMORY_BUS_H
#define DLX_MEMORY_BUS_H

#include <stdint.h>
#include "dlx_defs.h"

uint32_t dlx_memory_read_word(DLX_state *state, uint32_t address);

#endif // !DLX_MEMORY_BUS_H
