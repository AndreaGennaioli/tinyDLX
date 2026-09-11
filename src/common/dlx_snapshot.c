#include "dlx_snapshot.h"
#include "debug.h"
#include "dlx_defs.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

static const char *exec_state_name(DLX_exec_state s);

int dlx_snapshot_write(DLX_state *state, DLX_config *config) {
  if(state == NULL)
    return 0;
  if(config == NULL)
    return 0;
  if(config->dump_state == NULL)
    return 0;

  FILE *dump_file = fopen(config->dump_state, "w");

  if(dump_file == NULL) {
    error("Cannot open dump file '%s': %s", config->dump_state, strerror(errno));
    return 0;
  }

  fprintf(dump_file, "{\n");
  fprintf(dump_file, "  \"version\": 1,\n");
  fprintf(dump_file, "  \"state\": {\n");
  fprintf(dump_file, "    \"exit_reason\": \"%s\",\n", exec_state_name(state->exec_state));
  fprintf(dump_file, "    \"cycles\": \"%" PRIu64 "\",\n", state->cycles);
  fprintf(dump_file, "    \"pc\": \"0x%08X\",\n", state->pc);
  fprintf(dump_file, "    \"sr\": \"0x%08X\",\n", state->sr);
  fprintf(dump_file, "    \"iar\": \"0x%08X\",\n", state->iar);
  fprintf(dump_file, "    \"cr\": \"0x%08X\",\n", state->cr);
  
  size_t i;
  for (i = 0; i < DLX_GPR_COUNT-1; i++) {
    fprintf(dump_file, "    \"R%d\": \"0x%08X\",\n", (int) i, state->gpr[i]);
  }
  fprintf(dump_file, "    \"R%d\": \"0x%08X\"\n", (int) i, state->gpr[i]);
  fprintf(dump_file, "  }\n");
  fprintf(dump_file, "}\n");

  fclose(dump_file);
  return 1;
}

static const char *exec_state_name(DLX_exec_state s) {
  switch (s) {
    case DLX_RUNNING: return "running";
    case DLX_HALT:    return "halt";
    case DLX_FAULT:   return "fault";
    case DLX_SIGNAL:  return "signal";
    case DLX_TIMEOUT:  return "timeout";
  }
  return "unknown";

}
