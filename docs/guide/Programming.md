# Writing Programs for tinyDLX

For anyone writing programs that run on the bare machine. There is no operating system: the program is responsible for interrupt handling, stack management and device initialisation. The rules behind these conventions are described in the [architecture](../architecture/Overview.md) pages, and `tests/interrupt.asm` puts all of them together.

## Register conventions

| Register | Use |
|:--|:--|
| R0 | Always zero. |
| R26, R27 | Reserved to the interrupt handler, which needs them before it can save anything. The rest of the program must never use them. |
| R29 | Stack pointer. |
| R31 | Return address, written by `JAL` and `JALR`. |

R0 and R31 are fixed by the hardware, the others are conventions: the numbers are the ones MIPS uses for the same purposes. At reset no register other than R0 has a known value.

## Constants and addresses

`LHI` loads the upper half of a register and clears the lower one, then `ORI` sets the lower half. `ADDI` must not be used for the second step, because it sign-extends its immediate:

```asm
LHI   R1, 0x401F
ORI   R1, R1, 0xFFFC      ; R1 = 0x401FFFFC, with ADDI it would be 0x401EFFFC
```

Every device address has a zero lower half, so a single `LHI` loads it: `LHI R1, 0xC004` for the Input Port.

## Stack and procedures

The stack grows downwards from the last word of RAM. A procedure is called with `JAL` and returns with `JR R31`. It saves on the stack the registers it modifies, and R31 too if it calls another procedure:

```asm
PUTCHAR:                  ; writes the byte in R1 to the Output Port
  SUBI  R29, R29, 8       ; push R2 and R3
  SW    R2, 0(R29)
  SW    R3, 4(R29)
  LHI   R2, 0xC008        ; Output Port
PUTCHAR_WAIT:
  LBU   R3, 0(R2)         ; status: 1 ready, 0 busy
  BEQZ  R3, PUTCHAR_WAIT
  SB    R1, 0(R2)
  LW    R3, 4(R29)        ; pop R3 and R2
  LW    R2, 0(R29)
  ADDI  R29, R29, 8
  JR    R31
```

The loop on the status is required: a byte written while the Output Port is busy is lost.

## The code at address 0

Reset and every interrupt start at address 0. The first instructions read the Startup Circuit to tell them apart. On reset they clear it, set up the stack pointer and jump to the program:

```asm
LHI   R26, 0xC000         ; Startup Circuit
LB    R27, 0(R26)
BEQZ  R27, HANDLER        ; 0: entered on an interrupt
SB    R0, 0(R26)          ; 1: reset, clear the Startup Circuit
LHI   R29, 0x401F         ; stack pointer: last word of RAM
ORI   R29, R29, 0xFFFC
J     MAIN
```

Interrupts are enabled from the first instruction: an interrupt already pending at reset is taken before the Startup Circuit is cleared, is mistaken for reset, and leaves interrupts disabled. See [Interrupts.md](../architecture/Interrupts.md).

## Interrupt handlers

A handler saves the registers it uses, reads the Interrupt Controller to find the source, services that device, restores the registers and returns with `RFE`. Servicing means reading the device: the Interrupt Controller only records the request, and a level-triggered device keeps asserting it until it is read.

```asm
HANDLER:
  SUBI  R29, R29, 8       ; save what the handler uses
  SW    R1, 0(R29)
  SW    R31, 4(R29)
  LHI   R26, 0xC00C       ; Interrupt Controller
  LB    R27, 0(R26)       ; index of the pending line, 0xF if none
  BNEZ  R27, HANDLER_END  ; only line 0, the Input Port, is wired
  LHI   R26, 0xC004       ; Input Port
  LBU   R1, 0(R26)        ; reading the port withdraws its request
  JAL   PUTCHAR           ; echo the byte
HANDLER_END:
  LW    R31, 4(R29)
  LW    R1, 0(R29)
  ADDI  R29, R29, 8
  RFE
```

## Turning the machine off

A write of any value to the Power Manager turns the machine off:

```asm
LHI   R1, 0xC010          ; Power Manager
SB    R0, 0(R1)
```
