#include "dlx_debug.h"
#include "dlx_defs.h"
#include <ctype.h>
#include <stdio.h>

void dlx_dump_state(DLX_state *state) {
  if (state == NULL)
    return;

  fprintf(stderr, "\n=== STATE DUMP ==============================");
  fprintf(stderr, "\nPC: 0x%08X (%d)", state->pc, state->pc);
  for (uint8_t i = 0; i < 32; i++) {
    fprintf(stderr, "\nGPR %d: 0x%08X (%d)", i, state->gpr[i], state->gpr[i]);
  }
  fprintf(stderr, "\n=============================================\n");
}

static void dump_memory(uint8_t *memory, uint32_t start, uint32_t len) {
  for (uint32_t i = start; i < len; i += 16) {
    fprintf(stderr, "\n0x%08X:\t", i);

    for (int j = 0; j < 16; j++) {
      if (i + j < len)
        fprintf(stderr, " %02X ", memory[i + j]);
      else
        fprintf(stderr, "    ");
    }

    fprintf(stderr, " | ");

    for (int j = 0; j < 16; j++) {
      if (i + j < len) {
        uint8_t byte = memory[i + j];
        fprintf(stderr, "%c", isprint(byte) ? byte : '.');
      }
    }
  }
}

void dlx_dump_ram(DLX_state *state, uint32_t start, uint32_t len) {
  if (state == NULL || state->ram == NULL || len > DLX_RAM_SIZE)
    return;

  fprintf(stderr, "\n=== RAM DUMP [0x%08X - 0x%08X] ======", start,
          start + len);
  dump_memory(state->ram, start, len);
  fprintf(stderr, "\n=============================================\n");
}

void dlx_dump_rom(DLX_state *state, uint32_t start, uint32_t len) {
  if (state == NULL || state->rom == NULL || len > DLX_ROM_SIZE)
    return;

  fprintf(stderr, "\n=== ROM DUMP [0x%08X - 0x%08X] ======", start,
          start + len);
  dump_memory(state->rom, start, len);
  fprintf(stderr, "\n=============================================\n");
}
