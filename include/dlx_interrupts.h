#ifndef DLX_INTERRUPTS
#define DLX_INTERRUPTS

#include "dlx_defs.h"
#include <stdint.h>

// Executes debug interrups. See docs/Interrupts.md to see all the
// supported interrupt codes
void dlx_exec_debug_interrupt(uint32_t code, DLX_state *state);

#endif // !DLX_INTERRUPTS
