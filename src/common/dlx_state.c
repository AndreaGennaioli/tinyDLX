#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dlx_state.h"
#include "debug.h"

int dlx_state_init(DLX_state *state) {
  if(state == NULL) return 0;

  // Allocate and set to 0 the ROM
  state->rom = calloc(DLX_ROM_SIZE, sizeof(uint8_t));
  if(state->rom == NULL) {
    error("Failed to allocate %d bytes for DLX ROM.", DLX_ROM_SIZE);
    return 0;
  }

  // Allocate the RAM (random values are expected)
  state->ram = malloc(DLX_RAM_SIZE * sizeof(uint8_t));
  if(state->ram == NULL) {
    error("Failed to allocate %d bytes for DLX ram.", DLX_RAM_SIZE);

    free(state->rom);
    state->rom = NULL;
    return 0;
  }

  // Set pc to 0 (pc <- 0)
  state->pc = 0;

  // Random values for GPRs are expected, except for R0
  state->gpr[0] = 0;

  return 1;
}

void dlx_state_free(DLX_state *state) {
  if (state == NULL) return;

  if (state->rom != NULL) {
    free(state->rom);
    state->rom = NULL;
  }

  if (state->ram != NULL) {
    free(state->ram);
    state->ram = NULL;
  }
}
