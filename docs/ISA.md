# tinyDLX Instruction Set Architecture

## General information
All instructions are 32 bits long and there are 3 instruction types: R-Type, I-Type and J-Type.
The instructions can use wired integer values (immediate) and all the General Purpose Registers (R0..R31). The R0 register is hardwired to 0 and the R31 register is used to store the link address when using Jump-And-Link instructions.

### R-Type
| 31..26 (6) | 25..21 (5) | 20..16 (5) | 15..11 (5) | 10..6 (5) | 5..0 (6) |
|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|
| **Opcode** | **RS2** | **RS1** | **RD** | **Unused** | **Func** |

The R-Type instructions perform operations between registers. The **Opcode** is always zero and **Func** is used instead. **RS1** and **RS2** (register source 1 and 2) are the operands, **RD** is the register destination. **RS1**, **RS2** and **RD** are the numeric representation of any General Purpose Register.

### I-Type
| 31..26 (6) | 25..21 (5) | 20..16 (5) | 15..0 (16) |
|:----------:|:-----------:|:----------:|:----------:|
| **Opcode** | **RS2/RD** | **RS1** | **Immediate (Imm16)** |

The I-Type instructions perform operations between registers and immediate. **RS1** and **RS2** (register source 1 and 2) are the operands, RS1 can also serve as the **RD** (register destination). The **Immediate** is a 16 bit integer value which can be sign extended depending on the operation. I-Type instructions include Branch operations and Absolute Address Jumps (which use the address stored in RS1) also.

### J-Type
| 31..26 (6) | 25..0 (26) |
|:----------:|:----------:|
| **Opcode** | **Immediate (Imm26)** |

The J-Type instructions perform unconditional jumps to relative offset. RFE instruction is also included here.

## Instruction Set
| Mnemonic | Type | Opcode | Func | Explanation |
|:------|:------|:------|:------|:------|
| **SLL**   | R | 0x00 | 0x04 | R[rd] <- R[rs1] << (R[rs2] & 0x1F) |
| **SRL**   | R | 0x00 | 0x06 | R[rd] <- R[rs1] >> (R[rs2] & 0x1F) |
| **SRA**   | R | 0x00 | 0x07 | R[rd] <- R[rs1] >>> (R[rs2] & 0x1F) |
| **ADD**   | R | 0x00 | 0x20 | R[rd] <- R[rs1] + R[rs2] |
| **SUB**   | R | 0x00 | 0x22 | R[rd] <- R[rs1] - R[rs2] |
| **AND**   | R | 0x00 | 0x24 | R[rd] <- R[rs1] & R[rs2] |
| **OR**    | R | 0x00 | 0x25 | R[rd] <- R[rs1] \| R[rs2] |
| **XOR**   | R | 0x00 | 0x26 | R[rd] <- R[rs1] ^ R[rs2] |
| **SGT**   | R | 0x00 | 0x29 | R[rd] <- 0x01 if R[rs1] > R[rs2] else 0x00 |
| **SEQ**   | R | 0x00 | 0x2A | R[rd] <- 0x01 if R[rs1] = R[rs2] else 0x00 |
| **SGE**   | R | 0x00 | 0x2B | R[rd] <- 0x01 if R[rs1] >= R[rs2] else 0x00 |
| **SLT**   | R | 0x00 | 0x2C | R[rd] <- 0x01 if R[rs1] < R[rs2] else 0x00 |
| **SNE**   | R | 0x00 | 0x2D | R[rd] <- 0x01 if R[rs1] != R[rs2] else 0x00 |
| **SLE**   | R | 0x00 | 0x2E | R[rd] <- 0x01 if R[rs1] <= R[rs2] else 0x00 |
| **BEQZ**  | I | 0x04 | -    | if R[rs1] = 0 then PC <- PC + 4 + Imm16 |
| **BNEZ**  | I | 0x05 | -    | if R[rs1] != 0 then PC <- PC + 4 + Imm16 |
| **ADDI**  | I | 0x08 | -    | R[rd] <- R[rs1] + SignExt(Imm16) |
| **ADDUI** | I | 0x09 | -    | R[rd] <- R[rs1] + Imm16 |
| **SUBI**  | I | 0x0A | -    | R[rd] <- R[rs1] - SignExt(Imm16) |
| **SUBUI** | I | 0x0B | -    | R[rd] <- R[rs1] - Imm16 |
| **ANDI**  | I | 0x0C | -    | R[rd] <- R[rs1] & (0<sup>16</sup>##Imm16) |
| **ORI**   | I | 0x0D | -    | R[rd] <- R[rs1] \| (0<sup>16</sup>##Imm16) |
| **XORI**  | I | 0x0E | -    | R[rd] <- R[rs1] ^ (0<sup>16</sup>##Imm16) |
| **LHI**   | I | 0x0F | -    | R[rd] <- Imm16##0<sup>16</sup> |
| **JR**    | I | 0x12 | -    | PC <- R[rs1] |
| **JALR**  | I | 0x13 | -    | R[31] <- PC + 4; PC <- R[rs1] |
| **SLLI**  | I | 0x14 | -    | R[rd] <- R[rs1] << Imm16 |
| **SRLI**  | I | 0x15 | -    | R[rd] <- R[rs1] >> Imm16 |
| **SRAI**  | I | 0x17 | -    | R[rd] <- R[rs1] >>> Imm16 |
| **SGTI**  | I | 0x19 | -    | R[rd] <- 0x01 if R[rs1] > Imm16 else 0x00 |
| **SEQI**  | I | 0x1A | -    | R[rd] <- 0x01 if R[rs1] = Imm16 else 0x00 |
| **SGEI**  | I | 0x1B | -    | R[rd] <- 0x01 if R[rs1] >= Imm16 else 0x00 |
| **SLTI**  | I | 0x1C | -    | R[rd] <- 0x01 if R[rs1] < Imm16 else 0x00 |
| **SNEI**  | I | 0x1D | -    | R[rd] <- 0x01 if R[rs1] != Imm16 else 0x00 |
| **SLEI**  | I | 0x1E | -    | R[rd] <- 0x01 if R[rs1] <= Imm16 else 0x00 |
| **LB**    | I | 0x20 | -    | R[rd] <- SignExt(MEM[R[rs1] + Imm16]) |
| **LH**    | I | 0x21 | -    | R[rd] <- SignExt(MEM[R[rs1] + Imm16]) |
| **LW**    | I | 0x23 | -    | R[rd] <- MEM[R[rs1] + Imm16] |
| **LBU**   | I | 0x24 | -    | R[rd] <- (0<sup>24</sup>##MEM[R[rs1] + Imm16]) |
| **LHU**   | I | 0x25 | -    | R[rd] <- (0<sup>16</sup>##MEM[R[rs1] + Imm16]) |
| **SB**    | I | 0x28 | -    | MEM[R[rs2] + Imm16] <- R[rs1] |
| **SH**    | I | 0x29 | -    | MEM[R[rs2] + Imm16] <- R[rs1] |
| **SW**    | I | 0x2B | -    | MEM[R[rs2] + Imm16] <- R[rs1] |
| **J**     | J | 0x02 | -    | PC <- PC + 4 + Imm26 |
| **JAL**   | J | 0x03 | -    | R[31] <- PC + 4; PC <- PC + 4 + Imm26 |
| **INT**   | J | 0x39 | -    | Invoke interrupt handler with code Imm26 |
| **RFE**   | J | 0x3F | -    | PC <- IAR |
