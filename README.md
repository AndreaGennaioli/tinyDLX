# tinyDLX - DLX Processor Emulator

A didactic emulator for the DLX architecture (Hennessy & Patterson) written in C.

The current implementation is a **sequential core** (CPI = 5). A **pipelined core** implementation with hazard handling is planned.

## Project structure

- `bin/`: compiled files
- `docs/`: documentation (ISA reference, I/O, interrupts ...)
- `include/`: shared headers
- `obj/`: object files
- `src/common/`: common code (state, memory bus, loader ...)
- `src/cpu_seq/`: sequential core
- `src/devices/`: MMIO device implementations
- `tests/`: assembly test programs
- `tools/`: assembler (`asm.py`)

## Build

```bash
make
```

## Run
```bash
# Assemble a test program
python tools/asm.py tests/interrupt.asm tests/interrupt.bin

# Run it
./bin/cpu_seq -b tests/interrupt.bin
```

## Documentation

- [ISA reference](docs/ISA.md)
- [I/O and devices](docs/IO.md)
- [Interrupt system](docs/Interrupts.md)
- [Memory mappings](docs/Mappings.md)
- [Sequential core architecture](docs/seq/Architecture.md)
