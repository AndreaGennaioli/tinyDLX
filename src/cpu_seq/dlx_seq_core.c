#include "dlx_seq_core.h"
#include "debug.h"
#include "dlx_defs.h"
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

// Write masks for Special Purpose Registers.
static const uint32_t dlx_spr_wmask[DLX_SPR_COUNT] = {
  // Only IEN is writeble
  [DLX_SPR_SR]  = SR_IEN,
  // Full IAR is writable
  [DLX_SPR_IAR] = 0xFFFFFFFFu,
  // Cause Register is read-only
  [DLX_SPR_CR]  = 0,
};

void dlx_seq_step(DLX_state *state) {
  if (state == NULL)
    return;

  // Update devices tick
  for (uint32_t i = 0; i < state->device_count; i++) {
    if (state->devices[i]->tick != NULL)
      state->devices[i]->tick(state->devices[i]->state);
  }

  // Check if DLX interrupt line is asserted.
  // If so, disable interrupts (IEN = 0), set IAR to current pc,
  // set Cause Register to 0 (HW interrupt) and jump to address 0
  if(state->interrupt_line && (state->spr[DLX_SPR_SR] & SR_IEN)) {
    state->spr[DLX_SPR_SR] &= ~SR_IEN;
    state->spr[DLX_SPR_IAR] = state->pc;
    state->spr[DLX_SPR_CR] = 0;
    state->pc = 0;
  }

  // FETCH
  uint32_t raw_i = dlx_memory_read_word(state, state->pc);
  if(state->exec_state != DLX_FAULT) return;

  // DECODE
  state->pc += 4;
  decoded_instruction decoded_i;
  decode(raw_i, &decoded_i);


  // EXECUTE - MEMORY - WRITE BACK
  execute(state, &decoded_i);

  // Reset interrupt line
  // It will be reasserted by any device on tick (if needed)
  state->interrupt_line = 0;
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
  decoded_i->ra = (raw_i >> 21) & 0x1F;
  decoded_i->rb = (raw_i >> 16) & 0x1F;
  decoded_i->rc = (raw_i >> 11) & 0x1F;
  decoded_i->imm16 = raw_i & 0xFFFF;
  decoded_i->imm26 = raw_i & 0x3FFFFFF;
  decoded_i->imm16_sext = sign_extend_16(decoded_i->imm16);
  decoded_i->imm26_sext = sign_extend_26(decoded_i->imm26);
  decoded_i->func = raw_i & 0x3F;
}

static void execute(DLX_state *state, decoded_instruction *decoded_i) {
  uint32_t temp_ra;
  switch (decoded_i->opcode) {
  case I_RTYPE:
    switch (decoded_i->func) {
    case I_ADD_FUNC:
      state->gpr[decoded_i->rc] =
          state->gpr[decoded_i->ra] + state->gpr[decoded_i->rb];
      break;
    case I_SUB_FUNC:
      state->gpr[decoded_i->rc] =
          state->gpr[decoded_i->ra] - state->gpr[decoded_i->rb];
      break;
    case I_AND_FUNC:
      state->gpr[decoded_i->rc] =
          state->gpr[decoded_i->ra] & state->gpr[decoded_i->rb];
      break;
    case I_OR_FUNC:
      state->gpr[decoded_i->rc] =
          state->gpr[decoded_i->ra] | state->gpr[decoded_i->rb];
      break;
    case I_XOR_FUNC:
      state->gpr[decoded_i->rc] =
          state->gpr[decoded_i->ra] ^ state->gpr[decoded_i->rb];
      break;
    case I_SLL_FUNC:
      state->gpr[decoded_i->rc] = state->gpr[decoded_i->ra]
                                  << (state->gpr[decoded_i->rb] & 0x1F);
      break;
    case I_SRL_FUNC:
      state->gpr[decoded_i->rc] = (uint32_t)state->gpr[decoded_i->ra] >>
                                  (state->gpr[decoded_i->rb] & 0x1F);
      break;
    case I_SRA_FUNC:
      state->gpr[decoded_i->rc] = (int32_t)state->gpr[decoded_i->ra] >>
                                  (state->gpr[decoded_i->rb] & 0x1F);
      break;
    case I_SLT_FUNC:
      if ((int32_t)state->gpr[decoded_i->ra] <
          (int32_t)state->gpr[decoded_i->rb]) {
        state->gpr[decoded_i->rc] = 1;
      } else {
        state->gpr[decoded_i->rc] = 0;
      }
      break;
    case I_SLE_FUNC:
      if ((int32_t)state->gpr[decoded_i->ra] <=
          (int32_t)state->gpr[decoded_i->rb]) {
        state->gpr[decoded_i->rc] = 1;
      } else {
        state->gpr[decoded_i->rc] = 0;
      }
      break;
    case I_SGT_FUNC:
      if ((int32_t)state->gpr[decoded_i->ra] >
          (int32_t)state->gpr[decoded_i->rb]) {
        state->gpr[decoded_i->rc] = 1;
      } else {
        state->gpr[decoded_i->rc] = 0;
      }
      break;
    case I_SGE_FUNC:
      if ((int32_t)state->gpr[decoded_i->ra] >=
          (int32_t)state->gpr[decoded_i->rb]) {
        state->gpr[decoded_i->rc] = 1;
      } else {
        state->gpr[decoded_i->rc] = 0;
      }
      break;
    case I_SEQ_FUNC:
      if ((int32_t)state->gpr[decoded_i->ra] ==
          (int32_t)state->gpr[decoded_i->rb]) {
        state->gpr[decoded_i->rc] = 1;
      } else {
        state->gpr[decoded_i->rc] = 0;
      }
      break;
    case I_SNE_FUNC:
      if ((int32_t)state->gpr[decoded_i->ra] !=
          (int32_t)state->gpr[decoded_i->rb]) {
        state->gpr[decoded_i->rc] = 1;
      } else {
        state->gpr[decoded_i->rc] = 0;
      }
      break;
    case I_MOVI2S_FUNC:
      if(decoded_i->rc >= DLX_SPR_COUNT) {
        if(state->config->strict_mode) {
          error("EXECUTE: 0x%02X is not a Special Purpose Register", decoded_i->rc);
          state->exec_state = DLX_FAULT;
        } else {
          warn("EXECUTE: 0x%02X is not a Special Purpose Register", decoded_i->rc);
        }
      } else {
        // Write into SPR using its proper mask.
        state->spr[decoded_i->rc] = (state->gpr[decoded_i->ra] & dlx_spr_wmask[decoded_i->rc])
              | (state->spr[decoded_i->rc] & ~dlx_spr_wmask[decoded_i->rc]);
      }
      break;
    case I_MOVS2I_FUNC:
      if(decoded_i->ra >= DLX_SPR_COUNT) {
        if(state->config->strict_mode) {
          error("EXECUTE: 0x%02X is not a Special Purpose Register", decoded_i->ra);
          state->exec_state = DLX_FAULT;
        } else {
          warn("EXECUTE: 0x%02X is not a Special Purpose Register", decoded_i->ra);
        }
      } else {
        state->gpr[decoded_i->rc] = state->spr[decoded_i->ra];
      }
      break;
    default:
      if(state->config->strict_mode) {
        error("EXECUTE: 0x%02X not implemented R type function", decoded_i->func);
        state->exec_state = DLX_FAULT;
      } else {
        warn("EXECUTE: 0x%02X not implemented R type function", decoded_i->func);
      }
    }
    break;
  case I_ADDI:
    state->gpr[decoded_i->rb] =
        state->gpr[decoded_i->ra] + decoded_i->imm16_sext;
    break;
  case I_ADDUI:
    state->gpr[decoded_i->rb] = state->gpr[decoded_i->ra] + decoded_i->imm16;
    break;
  case I_SUBI:
    state->gpr[decoded_i->rb] =
        state->gpr[decoded_i->ra] - decoded_i->imm16_sext;
    break;
  case I_SUBUI:
    state->gpr[decoded_i->rb] = state->gpr[decoded_i->ra] - decoded_i->imm16;
    break;
  case I_SLLI:
    state->gpr[decoded_i->rb] = state->gpr[decoded_i->ra] << (decoded_i->imm16 & 0x1F);
    break;
  case I_SRLI:
    state->gpr[decoded_i->rb] =
        (uint32_t)state->gpr[decoded_i->ra] >> (decoded_i->imm16 & 0x1F);
    break;
  case I_SRAI:
    state->gpr[decoded_i->rb] =
        (int32_t)state->gpr[decoded_i->ra] >> (decoded_i->imm16 & 0x1F);
    break;
  case I_ORI:
    state->gpr[decoded_i->rb] =
        (int32_t)state->gpr[decoded_i->ra] | decoded_i->imm16;
    break;
  case I_ANDI:
    state->gpr[decoded_i->rb] =
        (int32_t)state->gpr[decoded_i->ra] & decoded_i->imm16;
    break;
  case I_XORI:
    state->gpr[decoded_i->rb] =
        (int32_t)state->gpr[decoded_i->ra] ^ decoded_i->imm16;
    break;
  case I_BNEZ:
    if (state->gpr[decoded_i->ra] != 0)
      state->pc = state->pc + decoded_i->imm16_sext;
    break;
  case I_BEQZ:
    if (state->gpr[decoded_i->ra] == 0)
      state->pc = state->pc + decoded_i->imm16_sext;
    break;
  case I_SLTI:
    if ((int32_t)state->gpr[decoded_i->ra] < decoded_i->imm16_sext) {
      state->gpr[decoded_i->rb] = 1;
    } else {
      state->gpr[decoded_i->rb] = 0;
    }
    break;
  case I_SLEI:
    if ((int32_t)state->gpr[decoded_i->ra] <= decoded_i->imm16_sext) {
      state->gpr[decoded_i->rb] = 1;
    } else {
      state->gpr[decoded_i->rb] = 0;
    }
    break;
  case I_SGTI:
    if ((int32_t)state->gpr[decoded_i->ra] > decoded_i->imm16_sext) {
      state->gpr[decoded_i->rb] = 1;
    } else {
      state->gpr[decoded_i->rb] = 0;
    }
    break;
  case I_SGEI:
    if ((int32_t)state->gpr[decoded_i->ra] >= decoded_i->imm16_sext) {
      state->gpr[decoded_i->rb] = 1;
    } else {
      state->gpr[decoded_i->rb] = 0;
    }
    break;
  case I_SEQI:
    if ((int32_t)state->gpr[decoded_i->ra] == decoded_i->imm16_sext) {
      state->gpr[decoded_i->rb] = 1;
    } else {
      state->gpr[decoded_i->rb] = 0;
    }
    break;
  case I_SNEI:
    if ((int32_t)state->gpr[decoded_i->ra] != decoded_i->imm16_sext) {
      state->gpr[decoded_i->rb] = 1;
    } else {
      state->gpr[decoded_i->rb] = 0;
    }
    break;
  case I_LHI:
    state->gpr[decoded_i->rb] = (uint32_t) decoded_i->imm16 << 16;
    break;
  case I_SW:
    dlx_memory_write_word(state,
                          state->gpr[decoded_i->ra] + decoded_i->imm16_sext,
                          state->gpr[decoded_i->rb]);
    break;
  case I_SH:
    dlx_memory_write_half_word(
        state, state->gpr[decoded_i->ra] + decoded_i->imm16_sext,
        state->gpr[decoded_i->rb] & 0xFFFF);
    break;
  case I_SB:
    dlx_memory_write_byte(state,
                          state->gpr[decoded_i->ra] + decoded_i->imm16_sext,
                          state->gpr[decoded_i->rb] & 0xFF);
    break;
  case I_LW:
    temp_ra = dlx_memory_read_word(state, state->gpr[decoded_i->ra] + decoded_i->imm16_sext);
    if(state->exec_state == DLX_RUNNING) {
      state->gpr[decoded_i->rb] = temp_ra;
    }
    break;
  case I_LHU:
    state->gpr[decoded_i->rb] = dlx_memory_read_half_word(
        state, state->gpr[decoded_i->ra] + decoded_i->imm16_sext);
    break;
  case I_LH:
    state->gpr[decoded_i->rb] = sign_extend_16(dlx_memory_read_half_word(
        state, state->gpr[decoded_i->ra] + decoded_i->imm16_sext));
    break;
  case I_LBU:
    state->gpr[decoded_i->rb] = dlx_memory_read_byte(
        state, state->gpr[decoded_i->ra] + decoded_i->imm16_sext);
    break;
  case I_LB:
    state->gpr[decoded_i->rb] = sign_extend_8(dlx_memory_read_byte(
        state, state->gpr[decoded_i->ra] + decoded_i->imm16_sext));
    break;
  case I_JR:
    state->pc = state->gpr[decoded_i->ra];
    break;
  case I_JALR:
    // Saving gpr[ra] prevents it to be overwrited in the link save (e.g. with JALR R31)
    temp_ra = state->gpr[decoded_i->ra];
    state->gpr[DLX_REG_LINK] = state->pc;
    state->pc = temp_ra;
    break;
  case I_J:
    state->pc = state->pc + decoded_i->imm26_sext;
    break;
  case I_JAL:
    state->gpr[DLX_REG_LINK] = state->pc;
    state->pc = state->pc + decoded_i->imm26_sext;
    break;
  case I_INT:
    if (decoded_i->imm26 >= 0xF0) {
      // Debug interrupts
      dlx_exec_debug_interrupt(decoded_i->imm26, state);
    } else if((state->spr[DLX_SPR_SR] & SR_IEN) && decoded_i->imm26 == 0x80) {
      state->spr[DLX_SPR_SR] &= ~SR_IEN;
      // the PC points to the next instruction
      state->spr[DLX_SPR_IAR] = state->pc;
      state->spr[DLX_SPR_CR] = 0x80;
      state->pc = 0;
    } else if (decoded_i->imm26 == 0x80) {
      error("EXECUTE: INT 0x80 with interrupts disabled; nested traps unsupported");
      state->exec_state = DLX_FAULT;
    } else {
      if(state->config->strict_mode) {
        error("EXECUTE: 0x%02X unknown interrupt", decoded_i->imm26);
        state->exec_state = DLX_FAULT;
      } else {
        warn("EXECUTE: 0x%02X unknown interrupt", decoded_i->imm26);
      }
    }
    break;
  case I_RFE:
    state->spr[DLX_SPR_SR] |= SR_IEN;
    state->pc = state->spr[DLX_SPR_IAR];
    break;
  default:
    if(state->config->strict_mode) {
      error("EXECUTE: 0x%02X not implemented instruction", decoded_i->opcode);
      state->exec_state = DLX_FAULT;
    } else {
      warn("EXECUTE: 0x%02X not implemented instruction", decoded_i->opcode);
    }
  }

  // R0 is wired to 0
  state->gpr[0] = 0;
}
