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
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

// Number of cycles between two synchronizations.
#define THROTTLE_INTERVAL 1000
// Nanoseconds in one second (used for conversions with frequency)
#define NS_PER_SEC 1000000000ULL

// Returns current time in ns
static uint64_t now_ns(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * NS_PER_SEC + ts.tv_nsec;
}

volatile sig_atomic_t quit = 0;

void handle_exit_signal(int signum) {
    quit = 1;
}

int main(int argc, char *argv[]) {
  DLX_config config = {.program_file = "\0"};
  DLX_state state;

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
  DLX_device *startup_circuit = dlx_startup_circuit_create(0xC0000000);
  if (startup_circuit == NULL) {
    error("Failed to create a device: Startup Circuit");
    dlx_exit(&state, EXIT_FAILURE);
  }
  dlx_device_register(&state, startup_circuit);
  info("Device created: Startup Circuit");

  DLX_device *ic = dlx_ic_create(0xC00C0000, DLX_MAX_DEVICES,
                                 state.assert_interrupt, &state);
  if (ic == NULL) {
    error("Failed to create a device: Interrupt Controller");
    dlx_exit(&state, EXIT_FAILURE);
  }
  dlx_device_register(&state, ic);
  info("Device created: Interrupt Controller");

  DLX_device *input_port = dlx_input_port_create(0xC0040000, ic->state, 0);
  if (input_port == NULL) {
    error("Failed to create a device: Input Port");
    dlx_exit(&state, EXIT_FAILURE);
  }
  dlx_device_register(&state, input_port);
  info("Device created: Input Port");

  DLX_device *output_port = dlx_output_port_create(0xC0080000);
  if (output_port == NULL) {
    error("Failed to create a device: Output Port");
    dlx_exit(&state, EXIT_FAILURE);
  }
  dlx_device_register(&state, output_port);
  info("Device created: Output Port");

  DLX_device *power_manager = dlx_power_manager_create(0xC0100000, &state);
  if (power_manager == NULL) {
    error("Failed to create a device: Power Manager");
    dlx_exit(&state, EXIT_FAILURE);
  }
  dlx_device_register(&state, power_manager);
  info("Device created: Power Manager");

  // Mounting program file
  // The program file is a binary file containing the program
  uint32_t program_size;
  if (dlx_load_program(&state, config.program_file, &program_size) == 0)
    return EXIT_FAILURE;

  info("Executing program...");

  // Cycle counter and start timestamp
  uint64_t cycle = 0;
  uint64_t start_ns = now_ns();

  // The sync interval is calculated from the frequency:
  //    ~100 synchronizations per second of simulated time.
  // Falls back to THROTTLE_INTERVAL in max-speed mode (freq_hz = 0).
  uint64_t sync_interval = (config.freq_hz > 0)
    ? ((uint64_t)config.freq_hz / 100 > 0 ? (uint64_t)config.freq_hz / 100 : 1)
    : THROTTLE_INTERVAL;
  while (!quit && state.pc < program_size) {
    dlx_seq_step(&state);
    cycle++;

    // Each interval of sync_interval cycles, the current elapsed time
    // it compared to the expected elapsed time based on the requested
    // frequency: if simulated_ns > elapsed_ns it means the emulator is running
    // faster than requested, so we wait; if elapsed_ns > simulated_ns it means
    // the emulator is running at max speed (100% use of core).
    if (config.freq_hz > 0 && cycle % sync_interval == 0) {
      uint64_t simulated_ns = (cycle * NS_PER_SEC) / config.freq_hz;
      uint64_t elapsed_ns   = now_ns() - start_ns;

      if (simulated_ns > elapsed_ns) {
        struct timespec sleep_ts = {
          .tv_sec  =  (simulated_ns - elapsed_ns) / NS_PER_SEC,
          .tv_nsec = ((simulated_ns - elapsed_ns) % NS_PER_SEC),
        };
        nanosleep(&sleep_ts, NULL);
      }
    }
  }

  dlx_exit(&state);
}
