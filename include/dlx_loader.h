#ifndef DLX_LOADER_H
#define DLX_LOADER_H

#include "dlx_defs.h"
#include <stdint.h>

// Load program file into memory.
// The program is loaded at 0x00000000, in the ROM
int dlx_load_program(DLX_state *state, const char *filename,
                     uint32_t *program_size);

#endif // !DLX_LOADER_H
