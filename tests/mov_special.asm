; ============================================================================
; Move Special
; ============================================================================
; This program aims to test MOVI2S and MOVS2I instructions
; Every register read back from an SPR expected to hold 0 is preloaded with a
; sentinel, so the snapshot tells a read apart from a register never written.
; ============================================================================

ADDUI R3, R0, 0xFFFF         ; Sentinel, overwritten below
ADDUI R5, R0, 0xFFFF         ; Sentinel, overwritten below

; SR: only IEN is writable, and it is 0 at reset
ADDUI  R1, R0, 0x65          ; R1 = 0x65 (IEN set, reserved bits set)
MOVI2S SR, R1                ; SR = 0x00000001 (IEN set, the rest dropped)
MOVS2I R2, SR                ; R2 = 0x00000001
ADDUI  R1, R0, 0x64          ; R1 = 0x64 (IEN clear)
MOVI2S SR, R1                ; SR = 0x00000000 (interrupts disabled again)
MOVS2I R3, SR                ; R3 = 0x00000000

; IAR: fully writable
MOVI2S IAR, R1               ; IAR = 0x00000064
MOVS2I R4, IAR               ; R4 = 0x00000064

; CR: read only
MOVI2S CR, R1                ; CR = 0x00000000 (write ignored)
MOVS2I R5, CR                ; R5 = 0x00000000

INT 0xF2                     ; Halt
