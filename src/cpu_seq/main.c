#include "debug.h"
#include "dlx_cli.h"
#include "dlx_debug.h"
#include "dlx_defs.h"
#include "dlx_loader.h"
#include "dlx_seq_core.h"
#include "dlx_state.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
  DLX_config config = {.program_file = "\0"};
  DLX_state state;

  if (parse_arguments(argc, argv, &config) == 0) {
    return EXIT_FAILURE;
  }

  info("Initializing DLX state");

  // Initialize DLX state
  if (dlx_state_init(&state) == 0)
    return EXIT_FAILURE;

  info("DLX state initialized");

  // Mounting program file
  // The program file is a binary file containing the program
  uint32_t program_size;
  if (dlx_load_program(&state, config.program_file, &program_size) == 0)
    return EXIT_FAILURE;

  info("Executing program...");

  // Arbitrary limit based on full_isa.asm test file
  while (state.pc < program_size) {
    dlx_seq_step(&state);
    getc(stdin);
  }

  info("Execution terminated");

  info("Exiting, bye bye...");

  return EXIT_SUCCESS;
}
