; ============================================================================
; Fault: write to ROM
; ============================================================================
; ROM is read only, and the program itself lives there.
; Expected: fault, exit code 1, "Write attempt to ROM at 0x00000000"
; Cycle cap: 100
; ============================================================================
SW R0, 0(R0)                 ; Faults: address 0 is the first word of ROM
