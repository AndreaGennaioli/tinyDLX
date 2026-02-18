; ============================================================================
; Full ISA test file
; ============================================================================
; This program aims to test all the ISA instructions.
; 42 out of 43 instruction are present, RFE is the one missing.
; The code assumes that:
; - RAM Base address: 0x40000000
; ============================================================================
ADDI R31, R0, 8

; Target instruction for JALR test
JR R31

; ============================================================================
; I-Type
; ============================================================================
ADDI  R1, R0, 10             ; R1 = 10 (0x0000000A)
ADDI  R2, R0, 0xFFFF         ; R2 = -1
ADDUI R2, R0, 0xFFFF         ; R2 = 0xFFFF
SUBI  R3, R2, 5              ; R3 = 0xFFFA
SUBUI R3, R2, -1             ; R3 = 0 (0xFFFF - 0xFFFF)
ADDI  R4, R0, -5             ; R4 = -5 (0xFFFFFFFB)

; I-Type Shifts
ADDI R1, R0, 0x1             ; R1 = 1 (2^0)
SLLI R1, R1, 0x4             ; R1 = 16 (2^4)
SRLI R1, R1, 0x2             ; R1 = 4 (2^2)
SRAI R1, R1, 0x1             ; R1 = 2 (2^1)
ADDI R1, R0, 0xFF00          ; R1 = 0xFFFFFF00
SRAI R1, R1, 0x4             ; R1 = 0xFFFFFFF0

; Test Logic Immediate
ORI  R5, R0, 0xF0            ; R5 = 0x000000F0
ANDI R5, R5, 0x30            ; R5 = 0x00000030 (0xF0 & 0x30)
XORI R5, R5, 0xFF            ; R5 = 0x000000CF (0x30 ^ 0xFF)

; Branch operations
ADDI R1, R0, 0               ; R1 = 0
BNEZ R1, IMPOSSIBLE_BRANCH   ; Should not jump (R1 = 0)
BEQZ R1, ALLWAYS_BRANCH      ; Should jump
IMPOSSIBLE_BRANCH:
ADDI R1, R0, 0xFFFF
BACK_BRANCH:
ADDI R1, R0, 0x1
ALLWAYS_BRANCH:
BEQZ R1, BACK_BRANCH         ; Should jump only the first time

; ============================================================================
; R-TYPE
; ============================================================================
ADDI R1, R0, 15
ADDI R2, R0, 7

ADD R3, R1, R2               ; R3 = 15 + 7 = 22
SUB R3, R1, R2               ; R3 = 15 - 7 = 8
AND R3, R1, R2               ; R3 = 15 & 7 = 0b1111 & 0b0111 = 0b0111 = 7
OR  R3, R1, R2               ; R3 = 15 | 7 = 15
XOR R3, R1, R2               ; R3 = 15 ^ 7 = 0b1000 = 8

; R-Type Shifts (Amount in Register)
ADDI R1, R0, 0x10
ADDI R11, R0, 4              ; R11 = 4 (Shift amount)
SLL  R12, R1, R11            ; R12 = 0x100 = 256
SRL  R12, R1, R11            ; R12 = 0x001
ADDI R1, R0, 0xFF00          ; R1 = 0xFFFFFF00
SRA  R12, R1, R11            ; R12 = 0xFFFFFFF0

; ============================================================================
; COMPARISON / SET (R-Type & I-Type)
; ============================================================================
ADDI R4, R0, -5
ADDI R1, R0, 10

; Immediate tests
SLTI R20, R4, 0              ; R20 = 1 (if R4 < 0)
SLEI R20, R4, 0              ; R20 = 1 (if R4 <= 0)
SGTI R20, R4, 0              ; R20 = 0 (if R4 > 0, but R4 < 0)
SGEI R20, R4, 0              ; R20 = 0 (if R4 >= 0, but R4 < 0)
SEQI R20, R1, 10             ; R20 = 1 (R1 = 10)
SNEI R20, R1, 10             ; R20 = 0

; Register tests
SLT  R21, R4, R1             ; R21 = 1 (-5 < 10)
SLE  R21, R4, R1             ; R21 = 1 (-5 <= 10)
SGT  R21, R1, R4             ; R21 = 1 (10 > -5)
SGE  R21, R1, R4             ; R21 = 1 (10 >= -5)
SEQ  R21, R1, R1             ; R21 = 1 (R1 = R1)
SNE R21, R1, R4              ; R21 = 1 (R1 != R4)

; ============================================================================
; MEMORY OPERATIONS (Load / Store)
; ============================================================================
; Base RAM Address into R10 (0x40000000)
LHI  R10, 0x4000             ; R10 = 0x40000000

ADDUI R1, R0, 0x800A
ADDI R5, R0, 0x00CF

; Store tests
SW R1, 0(R10)                ; Writes 0x0000000A at 0x40000000
SH R1, 4(R10)                ; Writes 0x000A at 0x40000004
SB R5, 8(R10)                ; Writes 0xCF at 0x40000008

; Load tests
LW  R22, 0(R10)              ; R22 = 0x0000800A
LHU R23, 4(R10)              ; R23 = 0x0000800A (Zero extended)
LH  R24, 4(R10)              ; R24 = 0xFFFF800A (Sign extended)
LBU R25, 8(R10)              ; R25 = 0x000000CF
LB  R26, 8(R10)              ; R26 = 0xFFFFFFCF (Sign extended byte)

; ============================================================================
; JUMP TESTS (J-Type & I-Type Register Jump)
; ============================================================================
ADDI R1, R0, 4
JALR R1
J JUMP_TARGET
ADDI R0, R0, 0               ; Should not be executed

JUMP_TARGET:
JAL FUNC_CALL                ; Jump and Link
J END_TEST                   ; Jump to the end

FUNC_CALL:
ADDI R27, R0, 1
JR R31                       ; Return to the caller

END_TEST:
ADDI R30, R0, 0xABC          ; Final instruction
