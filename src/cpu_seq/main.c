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
  (void) signum;
  quit = 1;
}

static int add_device(DLX_state *state, const char *name, DLX_device *dev);
static int setup_devices(DLX_state *state);

int main(int argc, char *argv[]) {
  DLX_config config = {.program_file = "\0", .freq_hz = 0, .max_cycles = 0};
  DLX_state state;

  signal(SIGINT, handle_exit_signal);
  signal(SIGTERM, handle_exit_signal);

  if (parse_arguments(argc, argv, &config) == 0) {
    return EXIT_FAILURE;
  }

  dlx_terminal_raw();

  info("Initializing DLX state");

  // Initialize DLX state
  if (dlx_state_init(&state, &config) == 0)
    return EXIT_FAILURE;

  info("DLX state initialized");

  // Initialize devices
  if (setup_devices(&state) == 0) {
    dlx_state_free(&state);
    return EXIT_FAILURE;
  }

  info("DLX devices initialized");

  // Mounting program file
  // The program file is a binary file containing the program
  if (dlx_load_program(&state, config.program_file) == 0) {
    dlx_state_free(&state);
    return EXIT_FAILURE;
  }

  info("Executing program...");

  // Start timestamp
  uint64_t start_ns = now_ns();

  // The sync interval is calculated from the frequency:
  //    ~100 synchronizations per second of simulated time.
  // Falls back to THROTTLE_INTERVAL in max-speed mode (freq_hz = 0).
  uint64_t sync_interval = (config.freq_hz > 0)
    ? ((uint64_t)config.freq_hz / 100 > 0 ? (uint64_t)config.freq_hz / 100 : 1)
    : THROTTLE_INTERVAL;
  while (state.exec_state == DLX_RUNNING) {
    if(quit) {
      state.exec_state = DLX_SIGNAL;
      break;
    }

    dlx_seq_step(&state);
    state.cycles++;

    if(state.config->max_cycles != 0 && state.cycles >= state.config->max_cycles) {
      state.exec_state = DLX_TIMEOUT;
      break;
    }

    // Each interval of sync_interval cycles, the current elapsed time
    // it compared to the expected elapsed time based on the requested
    // frequency: if simulated_ns > elapsed_ns it means the emulator is running
    // faster than requested, so we wait; if elapsed_ns > simulated_ns it means
    // the emulator is running at max speed (100% use of core).
    if (config.freq_hz > 0 && state.cycles % sync_interval == 0) {
      uint64_t simulated_ns = (state.cycles * NS_PER_SEC) / config.freq_hz;
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

  int code;
  switch(state.exec_state) {
    case DLX_HALT:   code = 0; break;
    case DLX_FAULT:  code = 1; break;
    case DLX_SIGNAL: code = 2; break;
    case DLX_TIMEOUT: code = 4; break;
    // Internal emulator error
    default:         code = 3; break;
  }

  fputc('\n', stderr);
  info("Execution terminated");

  dlx_state_free(&state);

  info("Exiting, bye bye...");

  return code;
}

static int add_device(DLX_state *state, const char *name, DLX_device *dev) {
  if(dev == NULL){
    error("Failed to create a device: %s", name);
    return 0;
  }

  if(!dlx_device_register(state, dev)) {
    dlx_device_destroy(dev);
    return 0;
  }

  info("Device registered: %s", name);
  return 1;
}

static int setup_devices(DLX_state *state) {
  DLX_device *ic = dlx_ic_create(0xC00C0000, DLX_MAX_DEVICES,
                                 state->assert_interrupt, state);

  if(!add_device(state, "Interrupt Controller", ic)
    || !add_device(state, "Startup Circuit", dlx_startup_circuit_create(0xC0000000))
    || !add_device(state, "Input Port", (ic ? dlx_input_port_create(0xC0040000, ic->state, 0) : NULL))
    || !add_device(state, "Output Port", dlx_output_port_create(0xC0080000))
    || !add_device(state, "Power Manager", dlx_power_manager_create(0xC0100000, state))
  ) {
    return 0;
  }

  return 1;
}
