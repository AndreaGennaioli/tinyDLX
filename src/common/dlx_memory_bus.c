#include "dlx_memory_bus.h"
#include "debug.h"
#include "dlx_defs.h"
#include <stdlib.h>

// Converts a DLX address into a real memory pointer
static uint8_t *get_phys_ptr(DLX_state *state, uint32_t address) {
  // Check ROM
  if (address >= DLX_ROM_BASE && address < (DLX_ROM_BASE + DLX_ROM_SIZE)) {
    return &state->rom[address - DLX_ROM_BASE];
  }

  // Check RAM
  if (address >= DLX_RAM_BASE && address < (DLX_RAM_BASE + DLX_RAM_SIZE)) {
    return &state->ram[address - DLX_RAM_BASE];
  }

  // Not mapped address or MMIO
  return NULL;
}

uint32_t dlx_memory_read_word(DLX_state *state, uint32_t address,
                              uint8_t convert_endianess) {
  if (state == NULL)
    return 0;

  if (address % 4 != 0) {
    warn("Unaligned read at 0x%08X", address);
  }

  // MMIO to be implemented here!

  // Getting the real pointer
  uint8_t *ptr = get_phys_ptr(state, address);

  // If convert_endianess = 1 convert from Big Endian to Little Endian:
  // everything stored in the DLX memory has to be in Big Endian,
  // but in order to use uint32_t a conversion is needed, since the eumulator
  // architecture is Little Endian.
  uint8_t b0 = convert_endianess ? ptr[0] : ptr[3];
  uint8_t b1 = convert_endianess ? ptr[1] : ptr[2];
  uint8_t b2 = convert_endianess ? ptr[2] : ptr[1];
  uint8_t b3 = convert_endianess ? ptr[3] : ptr[0];

  return (uint32_t)((b3 << 24) | (b2 << 16) | (b1 << 8) | b0);
}
