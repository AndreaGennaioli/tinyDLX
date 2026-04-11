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
// 0 ___________ 0x3FFFFFFF ___________ 0xC0000000 _________ 0xFFFFFFFF
//     ROM 64 KB     |       RAM 2 MB       |         MMIO

typedef struct DLX_ic_base {
  uint8_t lines[DLX_MAX_DEVICES];
  void (*assert_interrupt)(struct DLX_ic_base *ic, uint8_t index);
  void (*deassert_interrupt)(struct DLX_ic_base *ic, uint8_t index);
  void (*controller_assert_interrupt)(void* controller_state);
} DLX_ic_base;

typedef struct {
  uint32_t base_address;
  uint32_t range_size;
  void *state;
  DLX_ic_base *ic;
  void (*tick)(void *state);
  void (*free)(void *state);
  uint32_t (*read)(void *state, uint32_t offset, uint8_t bytes);
  void (*write)(void *state, uint32_t offset, uint32_t data, uint8_t bytes);
} DLX_device;

typedef enum {
    SR_IEN = 1 << 0,  // 00000001
} StatusRegisterFlags;

typedef struct {
  uint8_t *rom;
  uint8_t *ram;

  uint32_t gpr[DLX_GPR_COUNT];
  uint32_t pc;
  uint32_t device_count;

  // Status register bits:
  //  INDEX - DESC
  //    0   - IEN interrupt enable
  uint32_t sr;
  // Instruction Address Register
  // the register where the return address is stored before a interrupt
  // handling.
  uint32_t iar;

  DLX_device *devices[DLX_MAX_DEVICES];

  void (*assert_interrupt)(void *state);

  uint8_t interrupt_line;
} DLX_state;

typedef struct {
  char *program_file;
  uint32_t freq_hz;
} DLX_config;

#endif // !DLX_DEFS_H
