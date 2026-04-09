#include "dlx_seq_core.h"
#include "debug.h"
#include "dlx_interrupts.h"
#include "dlx_isa.h"
#include "dlx_memory_bus.h"
#include <unistd.h>

// Extend sign of immediate of 8 bits
static int32_t sign_extend_8(uint8_t imm8);
// Extend sign of immediate of 16 bits
static int32_t sign_extend_16(uint16_t imm16);
// Extend sign of immediate of 26 bits
static int32_t sign_extend_26(uint32_t imm26);
// Decode instruction into a struct
static void decode(uint32_t raw_i, decoded_instruction *decoded_i);
// Execute instruction
static void execute(DLX_state *state, decoded_instruction *decoded_i);

void dlx_seq_step(DLX_state *state) {
  if (state == NULL)
    return;

  // Update devices tick
  for (int i = 0; i < state->device_count; i++) {
    if (state->devices[i]->tick != NULL)
      state->devices[i]->tick(state->devices[i]->state);
  }

  // Check if DLX interrupt line is asserted.
  // If so, set IEN to 1, set IAR to current pc and jump to address 0
  if(state->interrupt_line && (state->sr & SR_IEN) == 0) {
    state->sr |= SR_IEN;
    state->iar = state->pc;
    state->pc = 0;
  }

  // FETCH
  // The program is expected to be in Big Endian, so no conversion is needed
  uint32_t raw_i = dlx_memory_read_word(state, state->pc, 0);

  // DECODE
  state->pc += 4;
  decoded_instruction decoded_i;
  decode(raw_i, &decoded_i);


  // EXECUTE - MEMORY - WRITE BACK
  execute(state, &decoded_i);

  // Reset interrupt line
  // It will be reasserted by any device on tick (if needed)
  state->interrupt_line = 0;

  // Sleep to avoid core overload
  usleep(1000);

}

static int32_t sign_extend_8(uint8_t imm8) {
  // Since imm8 is 8 bits long i can use this trick
  return (int32_t)(int8_t)imm8;
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

static void execute(DLX_state *state, decoded_instruction *decoded_i) {
  switch (decoded_i->opcode) {
  case 0:
    switch (decoded_i->func) {
    case I_ADD_FUNC:
      state->gpr[decoded_i->rd] =
          state->gpr[decoded_i->rs2] + state->gpr[decoded_i->rs1];
      break;
    case I_SUB_FUNC:
      state->gpr[decoded_i->rd] =
          state->gpr[decoded_i->rs1] - state->gpr[decoded_i->rs2];
      break;
    case I_AND_FUNC:
      state->gpr[decoded_i->rd] =
          state->gpr[decoded_i->rs2] & state->gpr[decoded_i->rs1];
      break;
    case I_OR_FUNC:
      state->gpr[decoded_i->rd] =
          state->gpr[decoded_i->rs2] | state->gpr[decoded_i->rs1];
      break;
    case I_XOR_FUNC:
      state->gpr[decoded_i->rd] =
          state->gpr[decoded_i->rs2] ^ state->gpr[decoded_i->rs1];
      break;
    case I_SLL_FUNC:
      state->gpr[decoded_i->rd] = state->gpr[decoded_i->rs1]
                                  << (state->gpr[decoded_i->rs2] & 0x1F);
      break;
    case I_SRL_FUNC:
      state->gpr[decoded_i->rd] = (uint32_t)state->gpr[decoded_i->rs1] >>
                                  (state->gpr[decoded_i->rs2] & 0x1F);
      break;
    case I_SRA_FUNC:
      state->gpr[decoded_i->rd] = (int32_t)state->gpr[decoded_i->rs1] >>
                                  (state->gpr[decoded_i->rs2] & 0x1F);
      break;
    case I_SLT_FUNC:
      if ((int32_t)state->gpr[decoded_i->rs1] <
          (int32_t)state->gpr[decoded_i->rs2]) {
        state->gpr[decoded_i->rd] = 1;
      } else {
        state->gpr[decoded_i->rd] = 0;
      }
      break;
    case I_SLE_FUNC:
      if ((int32_t)state->gpr[decoded_i->rs1] <=
          (int32_t)state->gpr[decoded_i->rs2]) {
        state->gpr[decoded_i->rd] = 1;
      } else {
        state->gpr[decoded_i->rd] = 0;
      }
      break;
    case I_SGT_FUNC:
      if ((int32_t)state->gpr[decoded_i->rs1] >
          (int32_t)state->gpr[decoded_i->rs2]) {
        state->gpr[decoded_i->rd] = 1;
      } else {
        state->gpr[decoded_i->rd] = 0;
      }
      break;
    case I_SGE_FUNC:
      if ((int32_t)state->gpr[decoded_i->rs1] >=
          (int32_t)state->gpr[decoded_i->rs2]) {
        state->gpr[decoded_i->rd] = 1;
      } else {
        state->gpr[decoded_i->rd] = 0;
      }
      break;
    case I_SEQ_FUNC:
      if ((int32_t)state->gpr[decoded_i->rs1] ==
          (int32_t)state->gpr[decoded_i->rs2]) {
        state->gpr[decoded_i->rd] = 1;
      } else {
        state->gpr[decoded_i->rd] = 0;
      }
      break;
    case I_SNE_FUNC:
      if ((int32_t)state->gpr[decoded_i->rs1] !=
          (int32_t)state->gpr[decoded_i->rs2]) {
        state->gpr[decoded_i->rd] = 1;
      } else {
        state->gpr[decoded_i->rd] = 0;
      }
      break;
    }
    break;
  case I_ADDI:
    state->gpr[decoded_i->rs2] =
        state->gpr[decoded_i->rs1] + decoded_i->imm16_sext;
    break;
  case I_ADDUI:
    state->gpr[decoded_i->rs2] = state->gpr[decoded_i->rs1] + decoded_i->imm16;
    break;
  case I_SUBI:
    state->gpr[decoded_i->rs2] =
        state->gpr[decoded_i->rs1] - decoded_i->imm16_sext;
    break;
  case I_SUBUI:
    state->gpr[decoded_i->rs2] = state->gpr[decoded_i->rs1] - decoded_i->imm16;
    break;
  case I_SLLI:
    state->gpr[decoded_i->rs2] = state->gpr[decoded_i->rs1] << decoded_i->imm16;
    break;
  case I_SRLI:
    state->gpr[decoded_i->rs2] =
        (uint32_t)state->gpr[decoded_i->rs1] >> decoded_i->imm16;
    break;
  case I_SRAI:
    state->gpr[decoded_i->rs2] =
        (int32_t)state->gpr[decoded_i->rs1] >> decoded_i->imm16;
    break;
  case I_ORI:
    state->gpr[decoded_i->rs2] =
        (int32_t)state->gpr[decoded_i->rs1] | decoded_i->imm16;
    break;
  case I_ANDI:
    state->gpr[decoded_i->rs2] =
        (int32_t)state->gpr[decoded_i->rs1] & decoded_i->imm16;
    break;
  case I_XORI:
    state->gpr[decoded_i->rs2] =
        (int32_t)state->gpr[decoded_i->rs1] ^ decoded_i->imm16;
    break;
  case I_BNEZ:
    if (state->gpr[decoded_i->rs1] != 0)
      state->pc = state->pc + decoded_i->imm16_sext;
    break;
  case I_BEQZ:
    if (state->gpr[decoded_i->rs1] == 0)
      state->pc = state->pc + decoded_i->imm16_sext;
    break;
  case I_SLTI:
    if ((int32_t)state->gpr[decoded_i->rs1] < decoded_i->imm16_sext) {
      state->gpr[decoded_i->rs2] = 1;
    } else {
      state->gpr[decoded_i->rs2] = 0;
    }
    break;
  case I_SLEI:
    if ((int32_t)state->gpr[decoded_i->rs1] <= decoded_i->imm16_sext) {
      state->gpr[decoded_i->rs2] = 1;
    } else {
      state->gpr[decoded_i->rs2] = 0;
    }
    break;
  case I_SGTI:
    if ((int32_t)state->gpr[decoded_i->rs1] > decoded_i->imm16_sext) {
      state->gpr[decoded_i->rs2] = 1;
    } else {
      state->gpr[decoded_i->rs2] = 0;
    }
    break;
  case I_SGEI:
    if ((int32_t)state->gpr[decoded_i->rs1] >= decoded_i->imm16_sext) {
      state->gpr[decoded_i->rs2] = 1;
    } else {
      state->gpr[decoded_i->rs2] = 0;
    }
    break;
  case I_SEQI:
    if ((int32_t)state->gpr[decoded_i->rs1] == decoded_i->imm16_sext) {
      state->gpr[decoded_i->rs2] = 1;
    } else {
      state->gpr[decoded_i->rs2] = 0;
    }
    break;
  case I_SNEI:
    if ((int32_t)state->gpr[decoded_i->rs1] != decoded_i->imm16_sext) {
      state->gpr[decoded_i->rs2] = 1;
    } else {
      state->gpr[decoded_i->rs2] = 0;
    }
    break;
  case I_LHI:
    state->gpr[decoded_i->rs2] = decoded_i->imm16 << 16;
    break;
  case I_SW:
    dlx_memory_write_word(state,
                          state->gpr[decoded_i->rs2] + decoded_i->imm16_sext,
                          state->gpr[decoded_i->rs1]);
    break;
  case I_SH:
    dlx_memory_write_half_word(
        state, state->gpr[decoded_i->rs2] + decoded_i->imm16_sext,
        state->gpr[decoded_i->rs1] & 0xFFFF);
    break;
  case I_SB:
    dlx_memory_write_byte(state,
                          state->gpr[decoded_i->rs2] + decoded_i->imm16_sext,
                          state->gpr[decoded_i->rs1] & 0xFF);
    break;
  case I_LW:
    state->gpr[decoded_i->rs1] = dlx_memory_read_word(
        state, state->gpr[decoded_i->rs2] + decoded_i->imm16_sext, 1);
    break;
  case I_LHU:
    state->gpr[decoded_i->rs1] = dlx_memory_read_half_word(
        state, state->gpr[decoded_i->rs2] + decoded_i->imm16_sext, 1);
    break;
  case I_LH:
    state->gpr[decoded_i->rs1] = sign_extend_16(dlx_memory_read_half_word(
        state, state->gpr[decoded_i->rs2] + decoded_i->imm16_sext, 1));
    break;
  case I_LBU:
    state->gpr[decoded_i->rs1] = dlx_memory_read_byte(
        state, state->gpr[decoded_i->rs2] + decoded_i->imm16_sext, 1);
    break;
  case I_LB:
    state->gpr[decoded_i->rs1] = sign_extend_8(dlx_memory_read_byte(
        state, state->gpr[decoded_i->rs2] + decoded_i->imm16_sext, 1));
    break;
  case I_JR:
    state->pc = state->gpr[decoded_i->rs1];
    break;
  case I_JALR:
    state->gpr[31] = state->pc;
    state->pc = state->gpr[decoded_i->rs1];
    break;
  case I_J:
    state->pc = state->pc + decoded_i->imm26_sext;
    break;
  case I_JAL:
    state->gpr[31] = state->pc;
    state->pc = state->pc + decoded_i->imm26_sext;
    break;
  case I_INT:
    if (decoded_i->imm26 >= 0xF0) {
      // Debug interrupts
      dlx_exec_debug_interrupt(decoded_i->imm26);
    } else {
      warn("EXECUTE: 0x%02X unknown interrupt", decoded_i->imm26);
    }
    break;
  case I_RFE:
    state->sr &= ~SR_IEN;
    state->pc = state->iar;
    break;
  default:
    warn("EXECUTE: 0x%02X not implemented instruction", decoded_i->opcode);
  }

  // R0 is wired to 0
  state->gpr[0] = 0;
}
