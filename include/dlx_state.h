#ifndef DLX_STATE_H
#define DLX_STATE_H

#include "dlx_defs.h"

// Initialize the DLX state (R[0] <- 0; PC <- 0; ROM[DLX_ROM_SIZE..0] <- 0)
// It allocates the memory.
int dlx_state_init(DLX_state *state);

// Frees all the allocated memory
void dlx_state_free(DLX_state *state);

// Allocates new device in state->devices
void dlx_device_register(DLX_state *state, DLX_device *device);

// Assert DLX interrupt line
void dlx_assert_interrupt(void *state);

// Exits the emulator
void dlx_exit(DLX_state *state);

#endif // !DLX_STATE_H
