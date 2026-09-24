; ============================================================================
; Fault: unknown debug interrupt code
; ============================================================================
; Codes from 0xF0 up belong to the emulator, but 0xF3 is not one it implements.
; Expected: fault, exit code 1, "EXECUTE: 0xF3 unknown debug interrupt code"
; Cycle cap: 100
; ============================================================================
INT 0xF3                     ; Faults: reserved debug code
