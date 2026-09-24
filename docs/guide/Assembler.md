# Assembler

For anyone writing tinyDLX assembly. The assembler is `tools/asm.py`; the instruction set it accepts is the one in [ISA.md](../architecture/ISA.md).

## Assembler invocation

```bash
python3 tools/asm.py input.asm output.bin
```

The output is a flat, big-endian binary, ready to be loaded in ROM. Every error message reports the line that caused it, with its number and its text.

## Source format

A line may start or end with spaces. Comments are introduced by `;` and run to the end of the line: there are no block comments. Blank lines are ignored.

Mnemonics and register names are case insensitive.

There are no directives: no `.data`, no `.org`, no way to reserve or initialise memory. Data has to be built at runtime.

### Labels

A label is written as `NAME:` on a line of its own. Names are case insensitive, cannot contain spaces and cannot be declared twice. A label used by a branch or a jump resolves to a PC relative displacement, a label used anywhere else resolves to its absolute address.

### Immediates

An immediate can be written in decimal, including negative values, in hexadecimal with `0x`, in octal with `0o` or in binary with `0b`. There are no character literals: a character is written as its ASCII code, such as `0x48` for `H`.

An immediate used as plain data is taken as a raw bit pattern, so in the 16 bit field of an I-Type instruction it spans from `-0x8000` to `0xFFFF`, and `-1` and `0xFFFF` describe the same field. A displacement stops at `0x7FFF` instead, because the hardware sign extends it: every larger pattern would read back as a negative number and send the jump the other way, so the assembler rejects it rather than encoding it. The same distinction applies to the 26 bit fields: data spans from `-0x2000000` to `0x3FFFFFF`, while a displacement stops at `0x1FFFFFF`.

`INT` takes an interrupt code, never a PC relative label.

### Special registers

`MOVI2S` and `MOVS2I` take their special register by name: `SR`, `IAR` or `CR`, case insensitive.
