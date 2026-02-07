#include "dlx_debug.h"
#include <ctype.h>
#include <stdio.h>

void dlx_dump_memory(DLX_state *state, uint32_t start, uint32_t len) {
  if (state == NULL || state->ram == NULL)
    return;

  fprintf(stderr, "\n=== MEMORY DUMP [0x%08X - 0x%08X] ===", start,
          start + len);

  for (uint32_t i = start; i < len; i += 16) {
    fprintf(stderr, "\n0x%08X:\t", i);

    for (int j = 0; j < 16; j++) {
      if (i + j < len)
        fprintf(stderr, " %02X ", state->ram[i + j]);
      else
        fprintf(stderr, "    ");
    }

    fprintf(stderr, " | ");

    for (int j = 0; j < 16; j++) {
      if (i + j < len) {
        uint8_t byte = state->ram[i + j];
        fprintf(stderr, "%c", isprint(byte) ? byte : '.');
      }
    }
  }

  fprintf(stderr, "\n=============================================\n");
}
