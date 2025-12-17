#ifndef DLX_DEFS_H
#define DLX_DEFS_H

#include <stdint.h>

#define DLX_GPR_COUNT 32  // General Purpose Register count
#define DLX_REG_ZERO  0   // R0: register wired to 0
#define DLX_REG_LINK  31  // R31: register use to save link address (JAL, JALR)
                          //
#define DLX_ROM_BASE  0
#define DLX_ROM_SIZE  (1024*64)     // 64 KB
#define DLX_RAM_BASE  0x40000000    // After 1 GB
#define DLX_RAM_SIZE  (1024*1024*2) // 2 MB

// Address space:
// 0 ___________ 0x3FFFFFFF ___________ 0xC0000000 _________ 0xFFFFFFFF
//     ROM 64 KB     |       RAM 2 MB       |         MMIO

typedef struct {
  uint32_t gpr[DLX_GPR_COUNT];
  uint32_t pc;

  uint8_t *rom;
  uint8_t *ram;
} DLX_state;

#endif // !DLX_DEFS_H
