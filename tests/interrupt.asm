; ============================================================================
; Interrupt test file
; ============================================================================
; This program aims to test devices and interrupts.
; 42 out of 43 instruction are present, RFE is the one missing.
; The code assumes that:
; - see docs/Mappings.md
; - R26 and R27 are reserved by the "kernel"
; ============================================================================

; Startup check
LHI R26, 0xC000
LB  R27, 0x0000(R26)

BEQZ R27, HANDLE_INTERRUPT

HANDLE_STARTUP:
  ; I will use R29 as the Stack Pointer
  LHI R29, 0x401F
  ADDI R29, R29, 0xFFFC

  LHI R26, 0xC000
  SB  R0, 0x0000(R26)        ; Dummy write to set SC to 0
  J MAIN

HANDLE_INTERRUPT:
  SUBI R29, R29, 16
  ; Push R1
  SW R1, 0(R29)
  ; Push R2
  SW R2, 4(R29)
  ; Push R3
  SW R3, 8(R29)
  ; Push R31
  SW R31, 12(R29)


  LHI R1, 0xC00C
  LB  R2, 0x0000(R1)
  ; Check for interrupt code 0 (Input port)
  BEQZ R2, HANDLE_INPUT_PORT
  ; Else back to main
  J EXIT_INTERRUPT_HANDLER

HANDLE_INPUT_PORT:
  LHI R1, 0xC004
  LBU  R3, 0x0000(R1)
  JAL PROC_OUTPUT
  J EXIT_INTERRUPT_HANDLER

EXIT_INTERRUPT_HANDLER:
  ; Pop R31
  LW R31, 12(R29)
  ; Pop R3
  LW R3, 8(R29)
  ; Pop R2
  LW R2, 4(R29)
  ; Pop R1
  LW R1, 0(R29)
  ADDI R29, R29, 16

  RFE

PROC_OUTPUT:
  ; OUTPUT procedure: R3 stores the byte to write
  ;     procedures need to be called by JAL
  SUBI R29, R29, 8
  ; Push R1
  SW R1, 0(R29)
  ; Push R2
  SW R2, 4(R29)

  LHI R1, 0xC008
CHECK_PORT_STATUS:
  LB R2, 0x0000(R1)

  BEQZ R2, CHECK_PORT_STATUS
  SB R3, 0x0000(R1)

  ; Pop R2
  LW R2, 4(R29)
  ; Pop R1
  LW R1, 0(R29)
  ADDI R29, R29, 8

  JR R31

MAIN:
  ADDI R3, R0, 0x21
MAIN_LOOP:                      ; Dummy loop
  JAL PROC_OUTPUT
  ADDI R3, R3, 1

  SUBI R4, R3, 0x7F
  BEQZ R4, MAIN

  J MAIN_LOOP

EXIT:
  ADD R0, R0, R0
