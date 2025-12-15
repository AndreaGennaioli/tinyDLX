#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dlx_state.h"
#include "debug.h"

int dlx_state_init(DLX_state *state) {
  if(state == NULL) return 0;

  // Allocate and set to 0 the memory
  state->memory = calloc(DLX_MEM_SIZE, sizeof(uint8_t));
  if(state->memory == NULL) {
    error("Failed to allocate %d bytes for DLX memory.", DLX_MEM_SIZE);
    return 0;
  }

  // Reset GPR, PC and memory
  return dlx_state_reset(state);
}

int dlx_state_reset(DLX_state *state) {
  if(state == NULL) return 0;

  // Reset GPR (R[31..0] <- 0)
  memset(state->gpr, 0, sizeof(state->gpr));

  // Set PC to 0 (PC <- 0)
  state->pc = 0;

  // Set memory to 0 if allocated (MEM[DLX_MEM_SIZE..0] <- 0)
  if(state->memory != NULL) {
    memset(state->memory, 0, DLX_MEM_SIZE);
  }

  return 1;
}

int dlx_state_cleanup(DLX_state *state) {
  if(state == NULL) return 0;

  // Free memory
  free(state->memory);

  state->memory = NULL;

  return 1;
}
