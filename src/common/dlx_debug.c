#include "dlx_debug.h"
#include "dlx_defs.h"
#include "dlx_isa.h"
#include <ctype.h>
#include <stdio.h>

void dlx_dump_state(DLX_state *state) {
  if (state == NULL)
    return;

  fprintf(stderr, "\n=== STATE DUMP ==============================");
  fprintf(stderr, "\nPC: 0x%08X (%d)", state->pc, state->pc);
  for (uint8_t i = 0; i < 32; i++) {
    fprintf(stderr, "\nGPR %d: 0x%08X (%d)", i, state->gpr[i], state->gpr[i]);
    if ((int32_t)state->gpr[i] < 0) {
      fprintf(stderr, " (%u)", state->gpr[i]);
    }
  }
  fprintf(stderr, "\n=============================================\n");
}

static void dump_memory(uint8_t *memory, uint32_t start, uint32_t len) {
  for (uint32_t i = start; i < len; i += 16) {
    fprintf(stderr, "\n0x%08X:\t", i);

    for (int j = 0; j < 16; j++) {
      if (i + j < len)
        fprintf(stderr, " %02X ", memory[i + j]);
      else
        fprintf(stderr, "    ");
    }

    fprintf(stderr, " | ");

    for (int j = 0; j < 16; j++) {
      if (i + j < len) {
        uint8_t byte = memory[i + j];
        fprintf(stderr, "%c", isprint(byte) ? byte : '.');
      }
    }
  }
}

void dlx_dump_ram(DLX_state *state, uint32_t start, uint32_t len) {
  if (state == NULL || state->ram == NULL || len > DLX_RAM_SIZE)
    return;

  fprintf(stderr, "\n=== RAM DUMP [0x%08X - 0x%08X] ======", start,
          start + len);
  dump_memory(state->ram, start, len);
  fprintf(stderr, "\n=============================================\n");
}

void dlx_dump_rom(DLX_state *state, uint32_t start, uint32_t len) {
  if (state == NULL || state->rom == NULL || len > DLX_ROM_SIZE)
    return;

  fprintf(stderr, "\n=== ROM DUMP [0x%08X - 0x%08X] ======", start,
          start + len);
  dump_memory(state->rom, start, len);
  fprintf(stderr, "\n=============================================\n");
}

void dlx_dump_decoded_instruction(decoded_instruction *decoded_i) {
  fprintf(stderr, "\n=== DECODED INSTRUCTION DUMP ======");

  fprintf(stderr, "\nOP: ");
  switch (decoded_i->opcode) {
  case 0:
    switch (decoded_i->func) {
    case I_SLL_FUNC:
      fprintf(stderr, "SLL");
      break;
    case I_SRL_FUNC:
      fprintf(stderr, "SRL");
      break;
    case I_SRA_FUNC:
      fprintf(stderr, "SRA");
      break;
    case I_ADD_FUNC:
      fprintf(stderr, "ADD");
      break;
    case I_SUB_FUNC:
      fprintf(stderr, "SUB");
      break;
    case I_AND_FUNC:
      fprintf(stderr, "AND");
      break;
    case I_OR_FUNC:
      fprintf(stderr, "OR");
      break;
    case I_XOR_FUNC:
      fprintf(stderr, "XOR");
      break;
    case I_SGT_FUNC:
      fprintf(stderr, "SGT");
      break;
    case I_SEQ_FUNC:
      fprintf(stderr, "SEQ");
      break;
    case I_SGE_FUNC:
      fprintf(stderr, "SGE");
      break;
    case I_SLT_FUNC:
      fprintf(stderr, "SLT");
      break;
    case I_SNE_FUNC:
      fprintf(stderr, "SNE");
      break;
    case I_SLE_FUNC:
      fprintf(stderr, "SLE");
      break;
    default:
      fprintf(stderr, "NOT_IMPLEMENTED_FUNC");
    }
    break;
  case I_BEQZ:
    fprintf(stderr, "BEQZ");
    break;
  case I_BNEZ:
    fprintf(stderr, "BNEZ");
    break;
  case I_ADDI:
    fprintf(stderr, "ADDI");
    break;
  case I_ADDUI:
    fprintf(stderr, "ADDUI");
    break;
  case I_SUBI:
    fprintf(stderr, "SUBI");
    break;
  case I_SUBUI:
    fprintf(stderr, "SUBUI");
    break;
  case I_ANDI:
    fprintf(stderr, "ANDI");
    break;
  case I_ORI:
    fprintf(stderr, "ORI");
    break;
  case I_XORI:
    fprintf(stderr, "XORI");
    break;
  case I_LHI:
    fprintf(stderr, "LHI");
    break;
  case I_JR:
    fprintf(stderr, "JR");
    break;
  case I_JALR:
    fprintf(stderr, "JALR");
    break;
  case I_SLLI:
    fprintf(stderr, "SLLI");
    break;
  case I_SRLI:
    fprintf(stderr, "SRLI");
    break;
  case I_SRAI:
    fprintf(stderr, "SRAI");
    break;
  case I_SGTI:
    fprintf(stderr, "SGTI");
    break;
  case I_SEQI:
    fprintf(stderr, "SEQI");
    break;
  case I_SGEI:
    fprintf(stderr, "SGEI");
    break;
  case I_SLTI:
    fprintf(stderr, "SLTI");
    break;
  case I_SNEI:
    fprintf(stderr, "SNEI");
    break;
  case I_SLEI:
    fprintf(stderr, "SLEI");
    break;
  case I_LB:
    fprintf(stderr, "LB");
    break;
  case I_LH:
    fprintf(stderr, "LH");
    break;
  case I_LW:
    fprintf(stderr, "LW");
    break;
  case I_LBU:
    fprintf(stderr, "LBU");
    break;
  case I_LHU:
    fprintf(stderr, "LHU");
    break;
  case I_SB:
    fprintf(stderr, "SB");
    break;
  case I_SH:
    fprintf(stderr, "SH");
    break;
  case I_SW:
    fprintf(stderr, "SW");
    break;
  case I_J:
    fprintf(stderr, "J");
    break;
  case I_JAL:
    fprintf(stderr, "JAL");
    break;
  case I_RFE:
    fprintf(stderr, "RFE");
    break;
  default:
    fprintf(stderr, "NOT_IMPLEMENTED");
  }

  if (decoded_i->rs2 >= 0 && decoded_i->rs2 < 32) {
    fprintf(stderr, "\nRS2: R%d", decoded_i->rs2);
  } else {
    fprintf(stderr, "\nRS2: -");
  }

  if (decoded_i->rs1 >= 0 && decoded_i->rs1 < 32) {
    fprintf(stderr, "\nRS1: R%d", decoded_i->rs1);
  } else {
    fprintf(stderr, "\nRS1: -");
  }

  if (decoded_i->rd >= 0 && decoded_i->rd < 32) {
    fprintf(stderr, "\nRD: R%d", decoded_i->rd);
  } else {
    fprintf(stderr, "\nRD: -");
  }
  fprintf(stderr, "\nImm16: %02X", decoded_i->imm16);
  fprintf(stderr, "\nImm26: %02X", decoded_i->imm26);
  fprintf(stderr, "\nImm16 SignExt: %02X", decoded_i->imm16_sext);
  fprintf(stderr, "\nImm16 SignExt: %02X", decoded_i->imm26_sext);
}
