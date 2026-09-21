# Testing

For anyone verifying that a change to the emulator did not break anything. There is no automated runner: the programs in `tests/` are run and checked by hand.

## Test programs

`tests/` holds only the `.asm` sources. The binaries are not versioned: assemble them after cloning and after every change to a source, by convention with the `.bin` extension.

```bash
python3 tools/asm.py tests/full_isa.asm tests/full_isa.bin
```

| Program | What it exercises |
|:--|:--|
| `full_isa.asm` | Every instruction of the ISA |
| `startup_circuit.asm` | Reading and clearing the Startup Circuit |
| `interrupt.asm` | Input and Output Port, Interrupt Controller and Power Manager |
| `stress.asm` | The speed of the emulator, timed with the debug timer |

## Checking a program

`full_isa.asm` and `startup_circuit.asm` write the expected value of each register in a comment next to the instruction that sets it. Run them from a known initial state, in strict mode, with a snapshot of the final state:

```bash
./bin/cpu_seq -b tests/full_isa.bin -G 0 -s -d full_isa.json
```

The exit code must be 0, and every register in the snapshot must hold the value of the last comment about it. Registers are reused along the program, so only the last value written to each one can be checked this way. The options are described in [Emulator.md](../guide/Emulator.md).

`interrupt.asm` echoes what is typed and turns the machine off on `q`. It can be checked without typing, by sending the input once the startup code has run:

```bash
(sleep 0.5; printf q) | ./bin/cpu_seq -b tests/interrupt.bin --freq 1000
```

The exit code must be 0. The delay matters: input already waiting at reset is taken before the startup code runs, as described in [Interrupts.md](../architecture/Interrupts.md).

`stress.asm` prints on standard error the time taken by about 8.6 billion instructions.
