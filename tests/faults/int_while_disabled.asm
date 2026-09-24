; ============================================================================
; Fault: INT 0x80 with interrupts disabled
; ============================================================================
; Nested traps are not supported, so a software interrupt raised while IEN is
; 0 stops the machine. This is the one fault that does not depend on strict
; mode. No setup is needed: interrupts are disabled at reset, and this program
; never enables them.
; Expected: fault, exit code 1, "EXECUTE: INT 0x80 with interrupts
; disabled; nested traps unsupported"
; Cycle cap: 100
; ============================================================================
INT 0x80                     ; Faults: no nested trap is possible
