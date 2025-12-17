#ifndef DLX_STATE_H
#define DLX_STATE_H

#include "dlx_defs.h"

// Initialize the DLX state (R[0] <- 0; PC <- 0; ROM[DLX_ROM_SIZE..0] <- 0)
// It allocates the memory.
int dlx_state_init(DLX_state *state);

// Frees all the allocated memory 
void dlx_state_free(DLX_state *state);

#endif // !DLX_STATE_H
