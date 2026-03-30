; ============================================================================
; Startup Circuit test file
; ============================================================================
; This program aims to test the startup circuit device wired to 0xC0000000
;   1. Read the state
;   2. Write to set it to 0
;   3. Read again to check
; ============================================================================
LHI R1, 0xC000
LB R2, 0x0000(R1) ; Should be 1

; Dummy write
ADDI R3, R0, 0x8988
SB R3, 0x0000(R1)

; Check if the FFD is updated
LB R2, 0x0000(R1) ; Should be 0
