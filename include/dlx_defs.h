#ifndef DLX_DEFS_H
#define DLX_DEFS_H

#include <stdint.h>

#define DLX_GPR_COUNT 32  // General Purpose Register count
#define DLX_REG_ZERO  0   // R0: register wired to 0
#define DLX_REG_LINK  31  // R31: register use to save link address (JAL, JALR)
#define DLX_MEM_SIZE  1024

typedef struct {
  uint32_t gpr[DLX_GPR_COUNT];
  uint32_t pc;

  uint8_t *memory;
} DLX_state;

#endif // !DLX_DEFS_H
