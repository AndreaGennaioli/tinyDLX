#ifndef DLX_DEBUG_H
#define DLX_DEBUG_H

#include "dlx_defs.h"
#include <stdint.h>

// Dumps memory to stderr
void dlx_dump_ram(DLX_state *state, uint32_t start, uint32_t len);
void dlx_dump_rom(DLX_state *state, uint32_t start, uint32_t len);

#endif // !DLX_DEBUG_H
