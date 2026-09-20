# tinyDLX Instruction Set Architecture

## General information

All instructions are 32 bits long and there are 3 instruction types: R-Type, I-Type and J-Type.
The instructions can use wired immediate values and all the General Purpose Registers (R0..R31). The R0 register is hardwired to 0 and the R31 register is used to store the link address when using Jump-And-Link instructions.

### Notation

To avoid confusion I've decided to name the register placeholders by their position: RA always lives in the bits 25..21, RB is always in 20..16, RC is always in 15..11 and replaces the canonical RD. It's only a naming convention used in the instruction decoding/execution.

### Labels

Every immediate used in any instruction can be set as a label. A label is a keyword in your code that points at a specific address in your code. All the labels are replaced with ad immediate by the assembler. To know more see [Assembler.md](../guide/Assembler.md).

### R-Type
| 31..26 (6) | 25..21 (5) | 20..16 (5) | 15..11 (5) | 10..6 (5) | 5..0 (6) |
|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|
| **Opcode** | **RA** | **RB** | **RC** | **Unused** | **Func** |

The R-Type instructions perform operations between registers. The **Opcode** is always zero and **Func** is used instead. **RA** is the first operand and **RB** is the second, **RC** is the destination. **RA**, **RB** and **RC** are the numeric representation of any General Purpose Register.

### I-Type
| 31..26 (6) | 25..21 (5) | 20..16 (5) | 15..0 (16) |
|:----------:|:-----------:|:----------:|:----------:|
| **Opcode** | **RA** | **RB** | **Immediate (Imm16)** |

The I-Type instructions perform operations between registers and immediate. **RA** is never written; in memory operations it holds the base address. **RB** is the destination register for ALU, set, shift, and load instructions, and holds the value to be stored in store instructions.

### J-Type
| 31..26 (6) | 25..0 (26) |
|:----------:|:----------:|
| **Opcode** | **Immediate (Imm26)** |

The J-Type instructions perform unconditional jumps to relative offset. RFE and INT instruction are also included here.

## Instruction Set
| Mnemonic | Type | Opcode | Func | Syntax | Explanation |
|:------|:------|:------|:------|:------|:------|
| **SLL**   | R | 0x00 | 0x04 | SLL rc, ra, rb | R[rc] <- R[ra] << (R[rb] & 0x1F) |
| **SRL**   | R | 0x00 | 0x06 | SRL rc, ra, rb | R[rc] <- R[ra] >> (R[rb] & 0x1F) |
| **SRA**   | R | 0x00 | 0x07 | SRA rc, ra, rb | R[rc] <- R[ra] >>> (R[rb] & 0x1F) |
| **ADD**   | R | 0x00 | 0x20 | ADD rc, ra, rb | R[rc] <- R[ra] + R[rb] |
| **SUB**   | R | 0x00 | 0x22 | SUB rc, ra, rb | R[rc] <- R[ra] - R[rb] |
| **AND**   | R | 0x00 | 0x24 | AND rc, ra, rb | R[rc] <- R[ra] & R[rb] |
| **OR**    | R | 0x00 | 0x25 | OR rc, ra, rb | R[rc] <- R[ra] \| R[rb] |
| **XOR**   | R | 0x00 | 0x26 | XOR rc, ra, rb | R[rc] <- R[ra] ^ R[rb] |
| **SGT**   | R | 0x00 | 0x29 | SGT rc, ra, rb | R[rc] <- 0x01 if R[ra] > R[rb] else 0x00 |
| **SEQ**   | R | 0x00 | 0x2A | SEQ rc, ra, rb | R[rc] <- 0x01 if R[ra] = R[rb] else 0x00 |
| **SGE**   | R | 0x00 | 0x2B | SGE rc, ra, rb | R[rc] <- 0x01 if R[ra] >= R[rb] else 0x00 |
| **SLT**   | R | 0x00 | 0x2C | SLT rc, ra, rb | R[rc] <- 0x01 if R[ra] < R[rb] else 0x00 |
| **SNE**   | R | 0x00 | 0x2D | SNE rc, ra, rb | R[rc] <- 0x01 if R[ra] != R[rb] else 0x00 |
| **SLE**   | R | 0x00 | 0x2E | SLE rc, ra, rb | R[rc] <- 0x01 if R[ra] <= R[rb] else 0x00 |
| **BEQZ**  | I | 0x04 | -    | BEQZ ra, Imm16 | if R[ra] = 0 then PC <- PC + 4 + SignExt(Imm16) |
| **BNEZ**  | I | 0x05 | -    | BNEZ ra, Imm16 | if R[ra] != 0 then PC <- PC + 4 + SignExt(Imm16) |
| **ADDI**  | I | 0x08 | -    | ADDI rb, ra, Imm16 | R[rb] <- R[ra] + SignExt(Imm16) |
| **ADDUI** | I | 0x09 | -    | ADDUI rb, ra, Imm16 | R[rb] <- R[ra] + ZeroExt(Imm16) |
| **SUBI**  | I | 0x0A | -    | SUBI rb, ra, Imm16 | R[rb] <- R[ra] - SignExt(Imm16) |
| **SUBUI** | I | 0x0B | -    | SUBUI rb, ra, Imm16 | R[rb] <- R[ra] - ZeroExt(Imm16) |
| **ANDI**  | I | 0x0C | -    | ANDI rb, ra, Imm16 | R[rb] <- R[ra] & (0<sup>16</sup>##Imm16) |
| **ORI**   | I | 0x0D | -    | ORI rb, ra, Imm16 | R[rb] <- R[ra] \| (0<sup>16</sup>##Imm16) |
| **XORI**  | I | 0x0E | -    | XORI rb, ra, Imm16 | R[rb] <- R[ra] ^ (0<sup>16</sup>##Imm16) |
| **LHI**   | I | 0x0F | -    | LHI rb, Imm16 | R[rb] <- Imm16##0<sup>16</sup> |
| **JR**    | I | 0x12 | -    | JR ra | PC <- R[ra] |
| **JALR**  | I | 0x13 | -    | JALR ra | R[31] <- PC + 4; PC <- R[ra] |
| **SLLI**  | I | 0x14 | -    | SLLI rb, ra, Imm16 | R[rb] <- R[ra] << (Imm16 & 0x1F)|
| **SRLI**  | I | 0x16 | -    | SRLI rb, ra, Imm16 | R[rb] <- R[ra] >> (Imm16 & 0x1F) |
| **SRAI**  | I | 0x17 | -    | SRAI rb, ra, Imm16 | R[rb] <- R[ra] >>> (Imm16 & 0x1F) |
| **SGTI**  | I | 0x19 | -    | SGTI rb, ra, Imm16 | R[rb] <- 0x01 if R[ra] > SignExt(Imm16) else 0x00 |
| **SEQI**  | I | 0x1A | -    | SEQI rb, ra, Imm16 | R[rb] <- 0x01 if R[ra] = SignExt(Imm16) else 0x00 |
| **SGEI**  | I | 0x1B | -    | SGEI rb, ra, Imm16 | R[rb] <- 0x01 if R[ra] >= SignExt(Imm16) else 0x00 |
| **SLTI**  | I | 0x1C | -    | SLTI rb, ra, Imm16 | R[rb] <- 0x01 if R[ra] < SignExt(Imm16) else 0x00 |
| **SNEI**  | I | 0x1D | -    | SNEI rb, ra, Imm16 | R[rb] <- 0x01 if R[ra] != SignExt(Imm16) else 0x00 |
| **SLEI**  | I | 0x1E | -    | SLEI rb, ra, Imm16 | R[rb] <- 0x01 if R[ra] <= SignExt(Imm16) else 0x00 |
| **LB**    | I | 0x20 | -    | LB rb, Imm16(ra) | R[rb] <- SignExt(MEM8[R[ra] + SignExt(Imm16)]) |
| **LH**    | I | 0x21 | -    | LH rb, Imm16(ra) | R[rb] <- SignExt(MEM16[R[ra] + SignExt(Imm16)]) |
| **LW**    | I | 0x23 | -    | LW rb, Imm16(ra) | R[rb] <- MEM32[R[ra] + SignExt(Imm16)] |
| **LBU**   | I | 0x24 | -    | LBU rb, Imm16(ra) | R[rb] <- (0<sup>24</sup>##MEM8[R[ra] + SignExt(Imm16)]) |
| **LHU**   | I | 0x25 | -    | LHU rb, Imm16(ra) | R[rb] <- (0<sup>16</sup>##MEM16[R[ra] + SignExt(Imm16)]) |
| **SB**    | I | 0x28 | -    | SB rb, Imm16(ra) | MEM8[R[ra] + SignExt(Imm16)] <- R[rb] |
| **SH**    | I | 0x29 | -    | SH rb, Imm16(ra) | MEM16[R[ra] + SignExt(Imm16)] <- R[rb] |
| **SW**    | I | 0x2B | -    | SW rb, Imm16(ra) | MEM32[R[ra] + SignExt(Imm16)] <- R[rb] |
| **J**     | J | 0x02 | -    | J Imm26 | PC <- PC + 4 + SignExt(Imm26) |
| **JAL**   | J | 0x03 | -    | JAL Imm26 | R[31] <- PC + 4; PC <- PC + 4 + SignExt(Imm26) |
| **INT**   | J | 0x39 | -    | INT Imm26 | Invoke interrupt handler with code Imm26 |
| **RFE**   | J | 0x3F | -    | RFE | PC <- IAR; SR[IEN]=1 |
