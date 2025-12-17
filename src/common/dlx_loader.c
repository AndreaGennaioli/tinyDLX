#include <stdio.h>
#include <stdlib.h>
#include "dlx_loader.h"
#include "debug.h"

int dlx_load_program(DLX_state *state, const char *filename) {
  if(state == NULL) return 0;

  FILE *fp = fopen(filename, "rb");

  if(fp == NULL) {
    error("Cannot open program file: %s", filename);
    return 0;
  }

  // Get file size
  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  rewind(fp);

  // Ensure program fit in memory
  if(file_size > DLX_ROM_SIZE) {
    error("Program too large: %ld bytes and ROM is %d bytes.", file_size, DLX_ROM_SIZE);
    fclose(fp);
    return 0;
  }

  // Read file and copy bytes into memory
  size_t read_bytes = fread(state->rom, 1, file_size, fp);

  // Check readed bytes
  if(read_bytes != file_size) {
    error("Reading Error: expected %ld bytes, got %lu", file_size, read_bytes);
    fclose(fp);
    return 0;
  }

  info("Loaded %ld bytes into ROM from %s", file_size, filename);

  fclose(fp);
  return 1;
}
