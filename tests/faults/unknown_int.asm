; ============================================================================
; Fault: unknown interrupt code
; ============================================================================
; 0x07 is neither the software interrupt 0x80 nor a debug code.
; Expected: fault, exit code 1, "EXECUTE: 0x07 unknown interrupt"
; Cycle cap: 100
; ============================================================================
INT 0x07                     ; Faults: the code is reserved
