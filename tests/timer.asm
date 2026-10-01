; ============================================================================
; Timer test file
; ============================================================================
; This program aims to test the timer device.
; The code assumes that:
; - see docs/architecture/Memory.md
; - see docs/architecture/Devices.md#Timer
; - R1 is incremented in the MAIN loop and checked during the automated test
; - R2 is used to count the numbers of timer interrupts
; ============================================================================

; Startup check
LHI R26, 0xC000
LB  R27, 0x0000(R26)

BEQZ R27, HANDLE_INTERRUPT

HANDLE_STARTUP:
  ; Startup Circuit
  LHI R26, 0xC000
  SB  R0, 0x0000(R26)        ; Dummy write to set SC to 0

  ; Enable interrupts
  ADDI   R27, R0, 1
  MOVI2S SR, R27

  ; Setup R1 and R2 for the test
  ADD R1, R0, R0
  ADD R2, R0, R0

  ; Set the timer period to 50
  ADDI R27, R0, 50
  LHI  R26, 0xC014
  SW R27, 0x0000(R26)

  J MAIN

HANDLE_INTERRUPT:
  ; Read IC
  LHI R26, 0xC00C
  LB  R27, 0x0000(R26)
  ; Check for interrupt code 1 (Timer)
  SUBUI R27, R27, 1
  BEQZ R27, HANDLE_TIMER
  ; Fail otherwise
  J FAIL

HANDLE_TIMER:
  ADDUI R2, R2, 1
  ; Ack the timer. The dummy read must return 1
  LHI   R26, 0xC014
  LW    R27, 0x0004(R26)
  SUBUI R27, R27, 1
  BNEZ  R27, FAIL

  ; Exit at the third timer interrupt
  SUBUI R27, R2, 3
  BEQZ R27, EXIT
  ; Else exit the interrupt handler

EXIT_INTERRUPT_HANDLER:
  RFE

MAIN:
  ADDUI R1, R1, 1
  J MAIN

EXIT:
  ; Debug HALT
  INT 0xF2

FAIL:
  ; Make the test fail triggering unknown interrupt code in strict mode
  INT 0xFF
