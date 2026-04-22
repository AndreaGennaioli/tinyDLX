#ifndef DLX_ISA_H
#define DLX_ISA_H

#include <stdint.h>

typedef struct {
  int32_t imm16_sext;
  int32_t imm26_sext;
  uint32_t imm26;
  uint16_t imm16;
  uint8_t opcode;
  uint8_t func;
  uint8_t rs1, rs2, rd;
} decoded_instruction;

// Follows all the instructions of the ISA.
// R-type instructions are identified by their function (func) code

#define I_SLL 0x00
#define I_SRL 0x00
#define I_SRA 0x00
#define I_ADD 0x00
#define I_SUB 0x00
#define I_AND 0x00
#define I_OR 0x00
#define I_XOR 0x00
#define I_SGT 0x00
#define I_SEQ 0x00
#define I_SGE 0x00
#define I_SLT 0x00
#define I_SNE 0x00
#define I_SLE 0x00
#define I_BEQZ 0x04
#define I_BNEZ 0x05
#define I_ADDI 0x08
#define I_ADDUI 0x09
#define I_SUBI 0x0A
#define I_SUBUI 0x0B
#define I_ANDI 0x0C
#define I_ORI 0x0D
#define I_XORI 0x0E
#define I_LHI 0x0F
#define I_JR 0x12
#define I_JALR 0x13
#define I_SLLI 0x14
#define I_SRLI 0x15
#define I_SRAI 0x17
#define I_SGTI 0x19
#define I_SEQI 0x1A
#define I_SGEI 0x1B
#define I_SLTI 0x1C
#define I_SNEI 0x1D
#define I_SLEI 0x1E
#define I_LB 0x20
#define I_LH 0x21
#define I_LW 0x23
#define I_LBU 0x24
#define I_LHU 0x25
#define I_SB 0x28
#define I_SH 0x29
#define I_SW 0x2B
#define I_J 0x02
#define I_JAL 0x03
#define I_RFE 0x3F
#define I_INT 0x39
#define I_SLL_FUNC 0x04
#define I_SRL_FUNC 0x06
#define I_SRA_FUNC 0x07
#define I_ADD_FUNC 0x20
#define I_SUB_FUNC 0x22
#define I_AND_FUNC 0x24
#define I_OR_FUNC 0x25
#define I_XOR_FUNC 0x26
#define I_SGT_FUNC 0x29
#define I_SEQ_FUNC 0x2A
#define I_SGE_FUNC 0x2B
#define I_SLT_FUNC 0x2C
#define I_SNE_FUNC 0x2D
#define I_SLE_FUNC 0x2E

#endif // !DLX_ISA_H
