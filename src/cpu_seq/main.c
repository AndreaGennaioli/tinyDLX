#include <stdlib.h>
#include "dlx_defs.h"
#include "dlx_state.h"
#include "debug.h"

int main(int argc, char *argv[]) {
  DLX_state state;

  info("Initializing DLX state");

  dlx_state_init(&state);

  info("DLX state initialized");

  return EXIT_SUCCESS;
}
