#include "dlx_seq_core.h"
#include "dlx_debug.h"
#include "dlx_memory_bus.h"
#include <stdlib.h>

// Extend sign of immediate of 16 bits
static int32_t sign_extend_16(uint16_t imm16);
// Extend sign of immediate of 26 bits
static int32_t sign_extend_26(uint32_t imm26);
// Decode instruction into a struct
static void decode(uint32_t raw_i, decoded_instruction *decoded_i);

void dlx_seq_step(DLX_state *state) {
  if (state == NULL || state->pc == 276)
    return;

  // FETCH
  uint32_t raw_i = dlx_memory_read_word(state, state->pc, 0);
  // TODO: add interrupt integration (INT signal and IEN flag)

  // DECODE
  state->pc += 4;
  decoded_instruction decoded_i;
  decode(raw_i, &decoded_i);

  // EXECUTE
  // dlx_dump_decoded_instruction(&decoded_i);
}

static int32_t sign_extend_16(uint16_t imm16) {
  // Since imm16 is 16 bits long i can use this trick
  return (int32_t)(int16_t)imm16;
}

static int32_t sign_extend_26(uint32_t imm26) {
  // Clear
  imm26 = imm26 & 0x03FFFFFF;

  // Check if negative (checking the MSB of imm26)
  if (imm26 & 0x02000000) {
    return (int32_t)(imm26 | 0xFC000000);
  }

  return (int32_t)imm26;
}

static void decode(uint32_t raw_i, decoded_instruction *decoded_i) {
  // See ISA.md
  decoded_i->opcode = (raw_i >> 26) & 0x3F;
  decoded_i->rs2 = (raw_i >> 21) & 0x1F;
  decoded_i->rs1 = (raw_i >> 16) & 0x1F;
  decoded_i->rd = (raw_i >> 11) & 0x1F;
  decoded_i->imm16 = raw_i & 0xFFFF;
  decoded_i->imm26 = raw_i & 0x3FFFFFF;
  decoded_i->imm16_sext = sign_extend_16(decoded_i->imm16);
  decoded_i->imm26_sext = sign_extend_26(decoded_i->imm26);
  decoded_i->func = raw_i & 0x3F;
}
