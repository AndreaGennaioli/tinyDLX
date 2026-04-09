#include "debug.h"
#include "devices/dlx_ic.h"
#include "devices/dlx_input_port.h"
#include "devices/dlx_output_port.h"
#include "devices/dlx_power_manager.h"
#include "devices/dlx_startup_circuit.h"
#include "dlx_cli.h"
#include "dlx_defs.h"
#include "dlx_loader.h"
#include "dlx_seq_core.h"
#include "dlx_state.h"
#include "dlx_terminal.h"
#include <stdlib.h>
#include <signal.h>

DLX_state *global_state = NULL;

void handle_exit_signal(int signum) {
  if (global_state != NULL) {
    info("Received exit signal");
    dlx_exit(global_state); 
  } else {
    dlx_terminal_restore();
    exit(EXIT_SUCCESS);
  }
}

int main(int argc, char *argv[]) {
  DLX_config config = {.program_file = "\0"};
  DLX_state state;

  global_state = &state;

  signal(SIGINT, handle_exit_signal);
  signal(SIGTERM, handle_exit_signal);

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
  dlx_device_register(
      &state, dlx_input_port_create(0xC0040000, ic->state, 0));
  info("Device created: Input Port");
  dlx_device_register(
      &state, dlx_output_port_create(0xC0080000));
  info("Device created: Output Port");
  dlx_device_register(
      &state, dlx_power_manager_create(0xC0100000, &state));
  info("Device created: Power Manager");

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

  dlx_exit(&state);
}
