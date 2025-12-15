#ifndef DLX_STATE_H
#define DLX_STATE_H

#include "dlx_defs.h"

// Initialize the DLX state (R[31..0] <- 0; PC <- 0; MEM[DLX_MEM_SIZE..0] <- 0)
// It allocates the memory.
// NOTE: theoretically, the only regester to be set to 0 on the startup
//       should be R0, however I've decided to set every register to 0
//       to help the asm programmer.
// TODO: add a flag or something to decide if the GPR should all be set to 0
int dlx_state_init(DLX_state *state);

// Reset the DLX state (R[31..0] <- 0; PC <- 0; MEM[DLX_MEM_SIZE..0] <- 0)
// It does not reallocate the memory.
int dlx_state_reset(DLX_state *state);

// Deallocate the memory.
int dlx_state_cleanup(DLX_state *state);

#endif // !DLX_STATE_H
