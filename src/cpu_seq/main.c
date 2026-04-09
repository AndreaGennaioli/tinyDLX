#include "debug.h"
#include "devices/dlx_ic.h"
#include "devices/dlx_startup_circuit.h"
#include "dlx_cli.h"
#include "dlx_debug.h"
#include "dlx_defs.h"
#include "dlx_loader.h"
#include "dlx_seq_core.h"
#include "dlx_state.h"
#include "dlx_terminal.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
  DLX_config config = {.program_file = "\0"};
  DLX_state state;

  if (parse_arguments(argc, argv, &config) == 0) {
    return EXIT_FAILURE;
  }

  dlx_terminal_raw();

  info("Initializing DLX state");

  // Initialize DLX state
  if (dlx_state_init(&state) == 0)
    return EXIT_FAILURE;

  info("DLX state initialized");

  // Initialize devices
  dlx_device_register(
      &state, dlx_startup_circuit_create(0xC0000000));
  info("Device created: Startup Circuit");
  DLX_device *ic =
      dlx_ic_create(0xC00C0000, DLX_MAX_DEVICES, state.assert_interrupt, &state);
  dlx_device_register(&state, ic);
  info("Device created: Interrupt Controller");

  // Mounting program file
  // The program file is a binary file containing the program
  uint32_t program_size;
  if (dlx_load_program(&state, config.program_file, &program_size) == 0)
    return EXIT_FAILURE;

  info("Executing program...");

  while (state.pc < program_size) {
    dlx_seq_step(&state);
    // getc(stdin);
  }

  info("Execution terminated");

  dlx_terminal_restore();

  info("Exiting, bye bye...");

  return EXIT_SUCCESS;
}
