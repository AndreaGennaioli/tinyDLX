#ifndef DLX_DEFS_H
#define DLX_DEFS_H

#include <stdint.h>

#define DLX_GPR_COUNT 32 // General Purpose Register count
#define DLX_REG_ZERO 0   // R0: register wired to 0
#define DLX_REG_LINK 31  // R31: register used to save link address

#define DLX_ROM_BASE 0
#define DLX_ROM_SIZE (1024 * 64)       // 64 KB
#define DLX_RAM_BASE 0x40000000        // After 1 GB
#define DLX_RAM_SIZE (1024 * 1024 * 2) // 2 MB

// MMIO Devices
#define DLX_MAX_DEVICES 16

// Address space:
// 0 ___________ 0x40000000___________ 0xC0000000 _________ 0xFFFFFFFF
//     ROM 64 KB     |       RAM 2 MB       |         MMIO

// Base struct for any interrup controller.
// This base struct needs to be included in the IC state struct.
typedef struct DLX_ic_base {
  // Interrupt lines for each device wired to the IC.
  // lines[3] == 1 means that the 4th interrupt line wired to the
  // IC has been asserted by a device.
  uint8_t lines[DLX_MAX_DEVICES];
  // The function that a device must use to assert an interrupt.
  // Needs to be implemented by the IC.
  void (*assert_interrupt)(struct DLX_ic_base *ic, uint8_t index);
  // The function that a device must use to deassert an interrupt.
  // Needs to be implemented by the IC.
  void (*deassert_interrupt)(struct DLX_ic_base *ic, uint8_t index);
  // The function that the IC must use to assert the interrupt line
  // of its controller (typically the CPU).
  void (*controller_assert_interrupt)(void* controller_state);
} DLX_ic_base;

// Struct for a device. tick(), free(), read() and write() must be implemented
// by the device.
typedef struct {
  uint32_t base_address;
  uint32_t range_size;
  // Custom device state
  void *state;
  // Interrupt Controller pointer, can be NULL
  DLX_ic_base *ic;
  // Executed at the start of each cycle
  void (*tick)(void *state);
  // Must free the state of the device
  void (*free)(void *state);
  uint32_t (*read)(void *state, uint32_t offset, uint8_t bytes);
  void (*write)(void *state, uint32_t offset, uint32_t data, uint8_t bytes);
} DLX_device;

typedef enum {
  // See docs or state.sr for documentation on SR flags

  // Interrupt Enable
  SR_IEN = 1 << 0,
} StatusRegisterFlags;

typedef struct {
  uint8_t *rom;
  uint8_t *ram;

  // General Purpose Registers
  uint32_t gpr[DLX_GPR_COUNT];
  // Program Counter
  uint32_t pc;
  // Status Register:
  //  INDEX - DESC
  //    0   - IEN: 1 = interrupts enabled, 0 = interrupts disabled
  uint32_t sr;
  // Instruction Address Register:
  // the register where the return address is stored before a interrupt
  // handling.
  uint32_t iar;
  // Cause Register:
  // contains the interrupt code of the last interrupt asserted.
  // Since the DLX is not capable of identifying the source of an hardware
  // interrupt, for HW interrupts the Cause Register is set to 0 (the program
  // has the task to comunicate with the Interrupt Controller), instead, for
  // SW interrupts (asserted using INT) the interrupt code is set into the
  // Cause Register.
  uint32_t cr;

  // Array containing all the devices. Every device contained in the array
  // can be mapped in a specified range of addresses, can define handlers
  // for write and read operations, can define a tick function executed at the
  // start of every cycle and can assert interrutps. Inter-device communication
  // is also possible with appropriate configuration.
  DLX_device *devices[DLX_MAX_DEVICES];
  uint32_t device_count;

  // Asserts the DLX's interrupt line. The state is voluntarily of void type.
  void (*assert_interrupt)(void *state);
  uint8_t interrupt_line;
} DLX_state;

// Configuration struct for the emulator.
// All the configurations can be set by command line arguments.
// The command line arguments are parsed by dlx_cli
typedef struct {
  // Filename of the program to load in the ROM. See dlx_loader
  char *program_file;
  uint32_t freq_hz;
} DLX_config;

#endif // !DLX_DEFS_H
