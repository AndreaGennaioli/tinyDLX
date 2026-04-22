#ifndef DLX_DEBUG_H
#define DLX_DEBUG_H

#include "dlx_defs.h"
#include "dlx_isa.h"
#include <stdint.h>

// Dumps DLX_state struct to stderr
void dlx_dump_state(DLX_state *state);
// Dumps RAM to stderr
void dlx_dump_ram(DLX_state *state, uint32_t start, uint32_t len);
// Dumps ROM to stderr
void dlx_dump_rom(DLX_state *state, uint32_t start, uint32_t len);
// Dumps decoded instruction to stderr
void dlx_dump_decoded_instruction(decoded_instruction *decoded_i);

#endif // !DLX_DEBUG_H
