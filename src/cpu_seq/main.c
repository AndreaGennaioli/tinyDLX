#include "debug.h"
#include "dlx_debug.h"
#include "dlx_defs.h"
#include "dlx_loader.h"
#include "dlx_seq_core.h"
#include "dlx_state.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <binary_file>\n", argv[0]);
    return EXIT_FAILURE;
  }

  DLX_state state;

  info("Initializing DLX state");

  // Initialize DLX state
  if (dlx_state_init(&state) == 0)
    return EXIT_FAILURE;

  info("DLX state initialized");

  // Mounting program file
  // The program file is a binary file containing the program
  if (dlx_load_program(&state, argv[1]) == 0)
    return 0;

  // Arbitrary limit based on full_isa.asm test file
  while (state.pc <= 276) {
    dlx_seq_step(&state);
    getc(stdin);
  }

  return EXIT_SUCCESS;
}
