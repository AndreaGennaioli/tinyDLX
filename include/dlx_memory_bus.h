#ifndef DLX_MEMORY_BUS_H
#define DLX_MEMORY_BUS_H

#include "dlx_defs.h"
#include <stdint.h>

uint32_t dlx_memory_read_word(DLX_state *state, uint32_t address);
uint32_t dlx_memory_read_half_word(DLX_state *state, uint32_t address);
uint32_t dlx_memory_read_byte(DLX_state *state, uint32_t address);

void dlx_memory_write_word(DLX_state *state, uint32_t address, uint32_t data);
void dlx_memory_write_half_word(DLX_state *state, uint32_t address,
                                uint16_t data);
void dlx_memory_write_byte(DLX_state *state, uint32_t address, uint8_t data);
#endif // !DLX_MEMORY_BUS_H
