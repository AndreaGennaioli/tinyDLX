# tinyDLX

A didactic emulator, written in C, of tinyDLX: a 32-bit RISC machine derived from the DLX architecture of Hennessy & Patterson.

tinyDLX is a small machine: an instruction set close to the textbook one, a flat memory with memory-mapped devices, a single interrupt line behind an interrupt controller. The project comes with its own assembler and a small test suite.

## Motivation

I got motivated to write this emulator while I was attending a course about computer architecture (Calcolatori Elettronici T) at the University of Bologna. I saw this project as an opportunity to learn more about emulation, architectures, and system programming.

I really enjoyed creating a coherent system that includes the emulator, the interrupt distribution system, the devices and the assembler.

It started with the simple goal of achieving the emulation of a small set of instructions. Now I see this project as a playground for my creativity.

## Features

- A DLX-derived instruction set: arithmetic, logic, set, shift, load/store, branch and jump instructions, plus software interrupts.
- A sequential core that executes one instruction per cycle.
- Memory-mapped devices: interrupt controller, startup circuit, input and output ports, power manager.
- An assembler with labels, written in Python.
- Options to control and inspect a run: cycle limit, register initialisation, a strict mode that turns warnings into faults, and a JSON snapshot of the final state.
- Execution at full speed or at a chosen frequency.
- A test suite: `make test` runs every test program (except the `stress.asm` benchmark) and compares its final state with a frozen "golden" snapshot.

## Quick start

Requirements: Linux, `gcc`, `make`, Python 3.

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

> See the transparency note on AI below.

## Repository layout

- `src/common/`: machine state, memory bus, loader, command line, snapshots
- `src/cpu_seq/`: the sequential core and the emulator entry point
- `src/devices/`: MMIO devices
- `include/`: headers
- `tools/`: the assembler and the test suite runner
- `tests/`: assembly test programs and the snapshots the test suite compares them with
- `docs/`: documentation

## Transparency note about the use of AI

I used LLMs (mainly Claude) to discuss some design choices, to review the code, to help me design a minimal and useful test suite and (since English is not my first language) to draft and edit the documentation, which I checked against the code. The code of the emulator and the assembler is my own work.
