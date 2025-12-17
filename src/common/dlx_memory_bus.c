#include <stdlib.h>
#include "dlx_defs.h"
#include "dlx_memory_bus.h"
#include "debug.h"

// Converts a DLX address into a real memory pointer
static uint8_t *get_phys_ptr(DLX_state *state, uint32_t address) {
  // Check ROM
  if (addr >= DLX_ROM_BASE && addr < (DLX_ROM_BASE + DLX_ROM_SIZE)) {
    return &state->rom[addr - DLX_ROM_BASE];
  }

  // Check RAM
  if (addr >= DLX_RAM_BASE && addr < (DLX_RAM_BASE + DLX_RAM_SIZE)) {
    return &state->ram[addr - DLX_RAM_BASE];
  }

  // Not mapped address or MMIO
  return NULL;
}

uint32_t dlx_memory_read_word(DLX_state *state, uint32_t address) {
  if(state == NULL) return 0;

  if(address % 4 != 0) {
    warn("Unaligned read at 0x%08X", addr);
  }

  // MMIO to be implemented here!

  // Getting the real pointer
  uint8_t *ptr = get_phys_ptr(state, address);

  // From Big Endian to Little Endian:
  // everything stored in the DLX memory has to be in Big Endian,
  // but in order to use uint32_t a conversion is needed
  uint8_t b0 = ptr[0];
  uint8_t b1 = ptr[1];
  uint8_t b2 = ptr[2];
  uint8_t b3 = ptr[3];

  return (uint32_t) ((b3 << 24) | (b2 << 16) | (b1 << 8) | b0);
}
