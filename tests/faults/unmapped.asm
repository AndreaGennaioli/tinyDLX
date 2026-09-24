; ============================================================================
; Fault: unmapped address
; ============================================================================
; 0x20000000 is past the end of ROM and below the RAM base: no memory and no
; device answers there.
; Expected: fault, exit code 1, "Read at unmapped address 0x20000000"
; Cycle cap: 100
; ============================================================================
LHI R1, 0x2000               ; R1 = 0x20000000, the hole between ROM and RAM
LW  R2, 0(R1)                ; Faults: nothing is mapped at that address
