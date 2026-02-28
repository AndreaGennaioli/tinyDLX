; ============================================================================
; Stress test program
; ============================================================================
; This program aims to stress the emulator with lots of operations.
; ============================================================================
LHI R1, 0xFFFF            ; Counter
ADDUI R1, R1, 0xFFFF

INT 0xF0

LOOP:
  SUBI R1, R1, 1
  BNEZ R1, LOOP

INT 0xF1

ADDI R1, R0, 42

