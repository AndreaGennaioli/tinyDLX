; ============================================================================
; Chronometer example program
; ============================================================================
; Prints an MM:SS chronometer to the Output Port, overwriting it in place
; (a Carriage Return precedes every update). Press 'q' to power off.
;
; Structure:
; - The interrupt handler is kept minimal. On a Timer interrupt it only
;   sets R2 = 1 and ACKs the Timer; on an Input Port interrupt it powers
;   off the machine if the key is 'q'.
; - MAIN busy waits on R2. When R2 is 1, UPDATE clears it, prints the
;   current time and advances the digits by one second.
;
; The code assumes that:
; - Device addresses come from lib/devices.inc
;   (memory map in docs/architecture/Memory.md)
; - The Timer counts clock cycles: one interrupt every FREQ cycles.
;   The display advances by one second per interrupt, so the chronometer
;   runs in real time only if the system clock runs at FREQ Hz
;   (10 kHz here). The handler is about 10 instructions long, well
;   below FREQ, so no expiration is lost.
;
; Register usage:
; - R2         program status: 0 = busy waiting, 1 = UPDATE pending
; - R10, R11   minute digits (tens, units)
; - R12, R13   second digits (tens, units)
; - R3         argument of PROC_OUTPUT (byte to write)
; - R31        return address of PROC_OUTPUT
; - R1, R4, R5 scratch registers of the main program
; - R26, R27   reserved to the interrupt handler (and to the startup code,
;              which runs with interrupts disabled). The main program must
;              never use them.
; ============================================================================

.include "../lib/devices.inc"
.equ FREQ_HI 0x0000
.equ FREQ_LO 0x2710    ; FREQ = 10000

; Startup check
LHI R26, STARTUP_CIRCUIT_HIGH
LB  R27, 0x0000(R26)

BEQZ R27, HANDLE_INTERRUPT

; Handle startup: disable the Startup Circuit, reset the clock, set the
; Timer period, enable interrupts, jump to MAIN
HANDLE_STARTUP:
  ; Startup Circuit
  LHI R26, STARTUP_CIRCUIT_HIGH
  SB  R0, 0x0000(R26)        ; Dummy write to set SC to 0

  ; Start with UPDATE to print initial clock
  ADDI R2, R0, 1

  ; Reset clock digits
  ADD R13, R0, R0     ; Second unit digit
  ADD R12, R0, R0     ; Second tens digit
  ADD R11, R0, R0     ; Minute unit digit
  ADD R10, R0, R0     ; Minute tens digit

  ; Set the timer period to FREQ
  LHI R27, FREQ_HI         ; R27 = 0x00010000
  ORI R27, R27, FREQ_LO    ; R27 = 0x000186A0 = 100000
  LHI  R26, TIMER_HIGH
  SW R27, 0x0000(R26)

  ; Enable interrupts
  ADDI   R27, R0, 1
  MOVI2S SR, R27

  J MAIN

; Handle interrupts
HANDLE_INTERRUPT:
  LHI R26, INTERRUPT_CONTROLLER_HIGH
  LB  R27, 0x0000(R26)
  ; Check for interrupt code 1 (Timer)
  SUBUI R26, R27, 1
  BEQZ R26, HANDLE_TIMER
  ; Check for interrupt code 0 (Input port)
  BEQZ R27, HANDLE_INPUT_PORT
  ; Else RFE
  J EXIT_INTERRUPT_HANDLER

HANDLE_INPUT_PORT:
  LHI R26, INPUT_PORT_HIGH
  LBU  R27, 0x0000(R26)
  SUBI R27, R27, 0x71             ; 0x71 = 'q'
  BEQZ R27, POWER_OFF
  J EXIT_INTERRUPT_HANDLER

HANDLE_TIMER:
  ; Set the program status to execute UPDATE
  ADDUI R2, R0, 1

  ; Dummy read to ACK the Timer
  LHI R26, TIMER_HIGH
  LW R27, 0x0004(R26)
  ; Fall-through to EXIT_INTERRUPT_HANDLER

EXIT_INTERRUPT_HANDLER:
  RFE

MAIN:
  BEQZ R2, MAIN   ; If R2 == 1 fall-through to UPDATE
UPDATE:
  ; Reset R2
  ADD R2, R0, R0

  ;;; Update the text
  ; Print Carriage Return
  ADDUI R3, R0, 0x0D
  JAL PROC_OUTPUT
  ; Print minute tens
  ADDUI R3, R10, 0x30     ; 0x30 = '0'
  JAL PROC_OUTPUT
  ; Print minute unit
  ADDUI R3, R11, 0x30     ; 0x30 = '0'
  JAL PROC_OUTPUT
  ; Print ':' separator
  ADDUI R3, R0, 0x3A     ; 0x3A = ':'
  JAL PROC_OUTPUT
  ; Print second tens
  ADDUI R3, R12, 0x30     ; 0x30 = '0'
  JAL PROC_OUTPUT
  ; Print second unit
  ADDUI R3, R13, 0x30     ; 0x30 = '0'
  JAL PROC_OUTPUT

  ;;; Update the time digits
  ; Increment the second unit digit
  ADDI R13, R13, 1
  SUBUI R1, R13, 10
  BNEZ R1, EXIT_UPDATE
  ; If R13 == 10 set to 0 and increment the second tens digit
  ADD R13, R0, R0
  ADDI R12, R12, 1
  SUBUI R1, R12, 6
  BNEZ R1, EXIT_UPDATE
  ; If R12 == 6 set to 0 and increment the minute unit digit
  ADD R12, R0, R0
  ADDI R11, R11, 1
  SUBUI R1, R11, 10
  BNEZ R1, EXIT_UPDATE
  ; If R11 == 10 set to 0 and increment the minute tens digit
  ADD R11, R0, R0
  ADDI R10, R10, 1
  SUBUI R1, R10, 6
  BNEZ R1, EXIT_UPDATE
  ; If 60 minutes has been reaced reset the clock resetting
  ; R10 to 0
  ADD R10, R0, R0

EXIT_UPDATE:
  J MAIN

PROC_OUTPUT:
  ; OUTPUT procedure: R3 stores the byte to write.
  ;        It overwrites R4 and R5.
  ;        Procedures need to be called by JAL.
  LHI R4, OUTPUT_PORT_HIGH
CHECK_PORT_STATUS:
  LB R5, 0x0000(R4)

  BEQZ R5, CHECK_PORT_STATUS
  SB R3, 0x0000(R4)

  JR R31

POWER_OFF:
  LHI R26, POWER_MANAGER_HIGH
  SB R0, 0x0000(R26)

