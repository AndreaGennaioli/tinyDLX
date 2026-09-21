# Your First Program

A tutorial: a complete tinyDLX program written from scratch, assembled and run. The program prints `Hello` and turns the machine off. The emulator must already be built, as in the Quick start of the [README](../../README.md), and every command is run from the root of the repository.

## The program

Save it as `hello.asm`:

```asm
; hello.asm: prints "Hello" and turns the machine off

; ---- address 0: reset and every interrupt start here
LHI   R26, 0xC000         ; Startup Circuit
LB    R27, 0(R26)
BEQZ  R27, HANDLER        ; 0: entered on an interrupt
SB    R0, 0(R26)          ; 1: reset, clear the Startup Circuit
LHI   R29, 0x401F         ; stack pointer: last word of RAM
ORI   R29, R29, 0xFFFC
J     MAIN

; ---- interrupts: the only source is the Input Port, and its input is discarded
HANDLER:
  LHI   R26, 0xC004       ; Input Port
  LBU   R27, 0(R26)       ; reading it withdraws the request
  RFE

; ---- writes the byte in R1 to the Output Port
PUTCHAR:
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

; ---- the program
MAIN:
  ADDI  R1, R0, 0x48      ; 'H'
  JAL   PUTCHAR
  ADDI  R1, R0, 0x65      ; 'e'
  JAL   PUTCHAR
  ADDI  R1, R0, 0x6C      ; 'l'
  JAL   PUTCHAR
  JAL   PUTCHAR           ; 'l' again: PUTCHAR leaves R1 unchanged
  ADDI  R1, R0, 0x6F      ; 'o'
  JAL   PUTCHAR
  ADDI  R1, R0, 0x0A      ; newline
  JAL   PUTCHAR
  LHI   R2, 0xC010        ; Power Manager
  SB    R0, 0(R2)         ; turns the machine off
```

## How it works

**Address 0.** Execution starts at address 0 on reset, and every interrupt jumps there as well. The first three instructions read the Startup Circuit to tell the two cases apart. On reset the program clears it, points the stack pointer R29 at the last word of RAM and jumps to `MAIN`.

**The handler.** The program never reads input, but a key pressed while it runs still raises an interrupt. The Input Port keeps requesting it until it is read, so the handler reads the byte and discards it before returning with `RFE`. It only uses R26 and R27, the registers reserved to the handler, so it has nothing to save.

**`PUTCHAR`.** Before writing a byte, the procedure waits for the Output Port to be ready: a byte written while the port is busy is lost. It restores every register it modifies and leaves R1 untouched, which is why `MAIN` can print the second `l` without loading R1 again.

**`MAIN`.** The assembler has no data directives and no character literals, so each character is loaded as its ASCII code and passed to `PUTCHAR` with `JAL`. A final write to the Power Manager turns the machine off.

## Assembling and running

```bash
python3 tools/asm.py hello.asm hello.bin
./bin/cpu_seq -b hello.bin --freq 100
```

At 100 Hz the characters appear one by one, and the whole run takes about a second. The emulator exits with code 0, the sign that the program turned the machine off.

## Checking the final state

Adding `--dump-state` writes the final state of the machine as JSON:

```bash
./bin/cpu_seq -b hello.bin --freq 100 -d hello.json
```

A few fields are enough to check that the program did what it should:

- `exit_reason` is `halt`: the program turned the machine off.
- `R1` is `0x0000000A`: the last character written, the newline.
- `R29` is `0x401FFFFC`: the stack pointer is back at the last word of RAM, so every push had its pop.

## Where to go next

- [Programming.md](./Programming.md) for the conventions in full, [Assembler.md](./Assembler.md) for the syntax and [Emulator.md](./Emulator.md) for every option of the emulator.
- The [architecture](../architecture/Overview.md) pages describe the machine the program runs on.
