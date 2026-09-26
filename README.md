# tinyDLX

A didactic emulator, written in C, of tinyDLX: a 32-bit RISC machine derived from the DLX architecture of Hennessy & Patterson.

tinyDLX codebase is small: an instruction set close to the textbook one, a flat memory with memory-mapped devices, a single interrupt line behind an interrupt controller. The project comes with its own assembler and a small test suite.

## Features

- A DLX-derived instruction set: arithmetic, logic, set, shift, load/store, branch and jump instructions, plus software interrupts.
- A sequential core that executes one instruction per cycle.
- Memory-mapped devices: interrupt controller, startup circuit, input and output ports, power manager.
- An assembler with labels, written in Python.
- Some useful options: cycle limit, register initialisation, a strict mode that turns warnings into faults, and a JSON snapshot of the final state.
- Execution at full speed or at a chosen frequency.
- A test suite: `make test` runs (almost) every test program and compares its final state with a frozen "golden" snapshot.

## Quick start

Requirements: Linux, a C compiler, `make`, Python 3.

```bash
make
python3 tools/asm.py tests/interrupt.asm tests/interrupt.bin
./bin/cpu_seq -b tests/interrupt.bin --freq 1000
```

The program prints the printable ASCII characters in a loop and echoes what you type; `q` turns the machine off, `Ctrl+C` stops the emulator. Run `./bin/cpu_seq --help` for all the options.

## Documentation

Start from the [documentation index](docs/README.md), or go straight to what you need:

- **Learn the machine**: [architecture overview](docs/architecture/Overview.md)
- **Write a program**: [your first program](docs/guide/First-Program.md)
- **Work on the emulator**: [the sequential core](docs/internals/Sequential-Core.md)

## Repository layout

- `src/common/`: machine state, memory bus, loader, command line, snapshots
- `src/cpu_seq/`: the sequential core and the emulator entry point
- `src/devices/`: MMIO devices
- `include/`: headers
- `tools/`: the assembler and the test suite runner
- `tests/`: assembly test programs and the snapshots the test suite compares them with
- `docs/`: documentation
