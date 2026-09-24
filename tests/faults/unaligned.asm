; ============================================================================
; Fault: unaligned access
; ============================================================================
; Word access at an address that is not a multiple of 4.
; Expected: fault, exit code 1, "Unaligned read at 0x00000001"
; Cycle cap: 100
; ============================================================================
ADDI R1, R0, 1               ; R1 = 0x00000001, inside ROM but misaligned
LW   R2, 0(R1)               ; Faults: word access must be 4 byte aligned
