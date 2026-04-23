#include "dlx_memory_bus.h"
#include "debug.h"
#include "dlx_defs.h"

static DLX_device *get_device(DLX_state *state, uint32_t address) {
  for (int i = 0; i < state->device_count; i++) {
    DLX_device *dev = state->devices[i];

    if (address >= dev->base_address &&
        address < dev->base_address + dev->range_size) {
      return dev;
    }
  }

  return NULL;
}

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

uint32_t dlx_memory_read_word(DLX_state *state, uint32_t address) {
  if (state == NULL)
    return 0;

  if (address % 4 != 0) {
    warn("Unaligned read at 0x%08X", address);
    return 0;
  }

  // MMIO
  DLX_device *dev = get_device(state, address);
  if (dev != NULL && dev->read != NULL) {
    return dev->read(dev->state, address - dev->base_address, 4);
  }

  // Getting the real pointer
  uint8_t *ptr = get_phys_ptr(state, address);
  if (ptr == NULL) {
    warn("Read at unmapped address 0x%08X", address);
    return 0;
  }

  // Memory is big-endian: byte at lowest address is the MSB.
  return ((uint32_t)ptr[0] << 24) | ((uint32_t)ptr[1] << 16)
       | ((uint32_t)ptr[2] << 8)  |  (uint32_t)ptr[3];
}

uint32_t dlx_memory_read_half_word(DLX_state *state, uint32_t address) {
  if (state == NULL)
    return 0;

  if (address % 2 != 0) {
    warn("Unaligned read at 0x%08X", address);
    return 0;
  }

  // MMIO
  DLX_device *dev = get_device(state, address);
  if (dev != NULL && dev->read != NULL) {
    return dev->read(dev->state, address - dev->base_address, 2);
  }

  // Getting the real pointer
  uint8_t *ptr = get_phys_ptr(state, address);
  if (ptr == NULL) {
    warn("Read at unmapped address 0x%08X", address);
    return 0;
  }

  // Memory is big-endian: byte at lowest address is the MSB.
  return ((uint32_t)ptr[0] << 8) | (uint32_t)ptr[1];
}

uint32_t dlx_memory_read_byte(DLX_state *state, uint32_t address) {
  if (state == NULL)
    return 0;

  // MMIO
  DLX_device *dev = get_device(state, address);
  if (dev != NULL && dev->read != NULL) {
    return dev->read(dev->state, address - dev->base_address, 1);
  }

  // Getting the real pointer
  uint8_t *ptr = get_phys_ptr(state, address);
  if (ptr == NULL) {
    warn("Read at unmapped address 0x%08X", address);
    return 0;
  }

  return (uint32_t)ptr[0];
}

void dlx_memory_write_word(DLX_state *state, uint32_t address, uint32_t data) {
  if (state == NULL)
    return;

  if (address % 4 != 0) {
    warn("Unaligned write at 0x%08X", address);
    return;
  }

  // ROM is read only
  if (address >= DLX_ROM_BASE && address < (DLX_ROM_BASE + DLX_ROM_SIZE)) {
    warn("Write attempt to ROM at 0x%08X", address);
    return;
  }

  // MMIO
  DLX_device *dev = get_device(state, address);
  if (dev != NULL && dev->write != NULL) {
    dev->write(dev->state, address - dev->base_address, data, 4);
    return;
  }

  // Getting real physic pointer
  uint8_t *ptr = get_phys_ptr(state, address);
  if (ptr == NULL) {
    warn("Write at unmapped address 0x%08X", address);
    return;
  }

  // Memory is big-endian: MSB at lowest address.
  ptr[0] = (data >> 24) & 0xFF;
  ptr[1] = (data >> 16) & 0xFF;
  ptr[2] = (data >> 8)  & 0xFF;
  ptr[3] =  data        & 0xFF;
}

void dlx_memory_write_half_word(DLX_state *state, uint32_t address,
                                uint16_t data) {
  if (state == NULL)
    return;

  if (address % 2 != 0) {
    warn("Unaligned write at 0x%08X", address);
    return;
  }

  // ROM is read only
  if (address >= DLX_ROM_BASE && address < (DLX_ROM_BASE + DLX_ROM_SIZE)) {
    warn("Write attempt to ROM at 0x%08X", address);
    return;
  }

  // MMIO
  DLX_device *dev = get_device(state, address);
  if (dev != NULL && dev->write != NULL) {
    dev->write(dev->state, address - dev->base_address, data, 2);
    return;
  }

  // Getting real physic pointer
  uint8_t *ptr = get_phys_ptr(state, address);
  if (ptr == NULL) {
    warn("Write at unmapped address 0x%08X", address);
    return;
  }

  // Memory is big-endian: MSB at lowest address.
  ptr[0] = (data >> 8) & 0xFF;
  ptr[1] =  data       & 0xFF;
}

void dlx_memory_write_byte(DLX_state *state, uint32_t address, uint8_t data) {
  if (state == NULL)
    return;

  // ROM is read only
  if (address >= DLX_ROM_BASE && address < (DLX_ROM_BASE + DLX_ROM_SIZE)) {
    warn("Write attempt to ROM at 0x%08X", address);
    return;
  }

  // MMIO
  DLX_device *dev = get_device(state, address);
  if (dev != NULL && dev->write != NULL) {
    dev->write(dev->state, address - dev->base_address, data, 1);
    return;
  }

  // Getting real physic pointer
  uint8_t *ptr = get_phys_ptr(state, address);
  if (ptr == NULL) {
    warn("Write at unmapped address 0x%08X", address);
    return;
  }

  ptr[0] = data & 0xFF;
}
