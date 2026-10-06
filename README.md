# tinyDLX

[![CI](https://github.com/AndreaGennaioli/tinyDLX/actions/workflows/ci.yml/badge.svg)](https://github.com/AndreaGennaioli/tinyDLX/actions/workflows/ci.yml)

A didactic emulator, written in C, of tinyDLX: a 32-bit RISC machine derived from the DLX architecture of Hennessy & Patterson.

<p align="center">
  <img src="docs/assets/chronometer.gif" alt="tinyDLX assembling and running a chronometer program" width="720">
  <br>
  <sub><code>examples/chronometer.asm</code>: a timer interrupt updates an MM:SS clock on the output port; <code>q</code> powers the machine off. Shown at 50× speed (<code>--freq 500000</code>, the program assumes a 10 kHz clock). Recorded with <a href="https://github.com/charmbracelet/vhs">VHS</a>.</sub>
</p>

tinyDLX is a small machine: an instruction set close to the textbook one, a flat memory with memory-mapped devices, a single interrupt line behind an interrupt controller. The project comes with its own assembler and a small test suite.

## Motivation

I got motivated to write this emulator while I was attending a course about computer architecture (Calcolatori Elettronici T) at the University of Bologna. I saw this project as an opportunity to learn more about emulation, architectures, and system programming.

I really enjoyed creating a coherent system that includes the emulator, the interrupt distribution system, the devices and the assembler.

It started with the simple goal of achieving the emulation of a small set of instructions. Now I see this project as a playground for my creativity.

## Features

- A DLX-derived instruction set: arithmetic, logic, set, shift, load/store, branch and jump instructions, plus software interrupts.
- A sequential core that executes one instruction per cycle.
- Memory-mapped devices: interrupt controller, startup circuit, input and output ports, power manager, programmable 32 bit timer.
- An assembler with labels, `.equ` and `.include` directives, written in Python.
- Options to control and inspect a run: cycle limit, register initialisation, a strict mode that turns warnings into faults, and a JSON snapshot of the final state.
- Execution at full speed or at a chosen frequency.
- A test suite: `make test` runs every test program (except the `stress.asm` benchmark) and compares its final state with a frozen "golden" snapshot.

## Limitations

These are the biggest limitations of the current design:

- Currently the whole system is driven by a sequential loop that ticks every device at every cycle. It does not implement any asynchronous event, in fact in the code there are some workarounds like tick timeouts to simulate this.
- The current test suite is small and only checks the final state of each program (see [Testing.md](./docs/internals/Testing.md)).

## Next steps

These are the next features I would like to implement, from the closest to the most distant in time:

- Add `.word`, `.half`, `.byte` and `.string` directives to the assembler.
- Add some example programs into an `examples/` directory (e.g. a time clock using the timer, Echo, Mini scheduler)
- Add an artificial random initialization of GPRs and RAM.
- `--input-tape <input string>` or `--input` to submit an input string to the input port and disable input from the terminal.
- Rewrite `dlx_memory_bus.c` to make it more readable and remove duplicated code.
- Privilege bit in the status register (0 = kernel, 1 = user).
- Dynamic component loading from shared libraries.
- A system configuration file with the possibility of specifying devices in use and the memory mapping.
- A better test suite: it should be able to check the execution at every cycle.
- Possibly, make the emulator event-driven.

## Quick start

Requirements: Linux, `gcc`, `make`, Python 3.

```bash
make
python3 tools/asm.py examples/chronometer.asm chronometer.bin
./bin/cpu_seq -b chronometer.bin --freq 10000
```

The program prints an MM:SS chronometer driven by Timer interrupts; `q` turns the machine off, `Ctrl+C` stops the emulator. Run `./bin/cpu_seq --help` for all the options.

## Documentation

Start from the [documentation index](docs/README.md), or go straight to what you need:

- **Learn the machine**: [architecture overview](docs/architecture/Overview.md)
- **Write a program**: [your first program](docs/guide/First-Program.md)
- **Work on the emulator**: [the sequential core](docs/internals/Sequential-Core.md)

If you prefer jumping straight to the code, the key files for understanding the emulator are [`main.c`](./src/cpu_seq/main.c), [`dlx_seq_core.c`](./src/cpu_seq/dlx_seq_core.c), [`dlx_ic.c`](./src/devices/dlx_ic.c).

> The documentation was written with the help of AI, [see below](#transparency-note-about-the-use-of-ai).

## Repository layout

- `src/common/`: machine state, memory bus, loader, command line, snapshots
- `src/cpu_seq/`: the sequential core and the emulator entry point
- `src/devices/`: MMIO devices
- `include/`: headers
- `tools/`: the assembler and the test suite runner
- `tests/`: assembly test programs and the snapshots the test suite compares them with
- `lib/`: `.inc` files included into tests and example programs
- `examples/`: example programs
- `docs/`: documentation

## Transparency note about the use of AI

I used LLMs (mainly Claude) to discuss some design choices, to review the code, to help me design and draft the test suite. Since English is not my first language, I also used them to draft and edit the documentation and some comments, which I checked against the code. The code of the emulator and the assembler is my own work.
