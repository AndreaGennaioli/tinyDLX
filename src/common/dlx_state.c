#include "dlx_state.h"
#include "dlx_terminal.h"
#include "debug.h"
#include <stdlib.h>
#include <string.h>

int dlx_state_init(DLX_state *state) {
  if (state == NULL)
    return 0;

  // Allocate and set to 0 the ROM
  state->rom = calloc(DLX_ROM_SIZE, sizeof(uint8_t));
  if (state->rom == NULL) {
    error("Failed to allocate %d bytes for DLX ROM.", DLX_ROM_SIZE);
    return 0;
  }

  // Allocate the RAM (random values are expected)
  state->ram = malloc(DLX_RAM_SIZE * sizeof(uint8_t));
  if (state->ram == NULL) {
    error("Failed to allocate %d bytes for DLX ram.", DLX_RAM_SIZE);

    free(state->rom);
    state->rom = NULL;
    return 0;
  }

  // Set pc to 0 (pc <- 0)
  state->pc = 0;

  // Random values for GPRs are expected, except for R0
  state->gpr[0] = 0;

  state->device_count = 0;

  state->sr = 0;
  state->iar = 0;
  state->interrupt_line = 0;
  state->assert_interrupt = dlx_assert_interrupt;

  return 1;
}

void dlx_device_register(DLX_state *state, DLX_device *device) {
  if (state->device_count >= DLX_MAX_DEVICES) {
    error("Too many devices: max %d", DLX_MAX_DEVICES);
    return;
  }
  state->devices[state->device_count] = device;
  state->device_count++;
}

void dlx_state_free(DLX_state *state) {
  if (state == NULL)
    return;

  if (state->rom != NULL) {
    free(state->rom);
    state->rom = NULL;
  }

  if (state->ram != NULL) {
    free(state->ram);
    state->ram = NULL;
  }

  for (int i = 0; i < state->device_count; i++) {
    if(state->devices[i]->free != NULL) {
      state->devices[i]->free(state->devices[i]->state);
    }
    free(state->devices[i]);
  }
}

void dlx_assert_interrupt(void *state) {
  ((DLX_state *)state)->interrupt_line = 1;
}

void dlx_exit(DLX_state *state) {
  info("Execution terminated");

  dlx_state_free(state);

  info("Exiting, bye bye...");

  exit(EXIT_SUCCESS);
}
