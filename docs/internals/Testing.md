# Testing

For anyone verifying that a change to the emulator did not break anything. `make test` runs the whole suite: every case is compared, register by register, with a snapshot frozen from a run that was known to be correct.

## Running the suite

```bash
make test                    # rebuilds the emulator, then runs every case
tools/run_tests.sh           # the same suite, without the rebuild
tools/run_tests.sh --update  # update the 'golden state' snapshots
```

Every case is built in a temporary directory and run from a known initial state: GPRs at 0 (`-G 0`), strict mode (`-s`) and a cycle cap. The runner then compares the exit code and the final snapshot with the expected one under `tests/expected/`, and a failure prints the lines that differ, so a broken instruction is named by the registers it got wrong.

Standard input is `/dev/null` unless the case has an `.in` file next to its source, which is fed to the program instead. Only `interrupt.asm` has one, `tests/interrupt.in`.

## Updating the expected snapshots

```bash
tools/run_tests.sh --update
```

The expected files are rewritten instead of being compared. The exit code of each case is still checked, so a run that faults when it should halt is reported and its expected file left alone.

## Test programs

`tests/` holds the `.asm` sources, the programs that are expected to generate a fault are under `faults/` and the frozen snapshots under `expected/`, one `<name>.json` per case. The runner assembles its test files in a temporary directory.

The program binaries should end with `.bin` by convention.

```bash
python3 tools/asm.py tests/full_isa.asm tests/full_isa.bin
```

| Program | What it exercises |
|:--|:--|
| `full_isa.asm` | Every instruction of the ISA |
| `mov_special.asm` | Reading and writing the special registers, write masks included |
| `startup_circuit.asm` | Reading and clearing the Startup Circuit |
| `faults/*.asm` | One fault condition each: unaligned access, write to ROM, unmapped address, unknown interrupt code, `INT 0x80` with interrupts disabled, unknown debug code |
| `interrupt.asm` | Input and Output Port, Interrupt Controller and Power Manager, driven by the two bytes in `interrupt.in` |
| `stress.asm` | The speed of the emulator, timed with the debug timer |

Three cases have no source at all: an unassigned opcode, an unassigned R-Type function code and a special register number above 2. They are reserved encodings the assembler refuses to emit, so the runner builds each one as a single 32 bit word.

## Checking a program by hand

A program can always be run on its own, for example:

```bash
printf q | ./bin/cpu_seq -b tests/interrupt.bin
```

The options used to run a program on its own are described in [Emulator.md](../guide/Emulator.md).
