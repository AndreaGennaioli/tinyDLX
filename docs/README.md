# tinyDLX documentation

The documentation is organised by audience: what the machine is, how to write programs for it, and how the emulator that runs it is built.

## Architecture

tinyDLX described as a machine, as the programmer sees it, independently of how it is emulated.

| Page | Contents |
|:--|:--|
| [Overview.md](./architecture/Overview.md) | What tinyDLX is, and how it differs from DLX |
| [ISA.md](./architecture/ISA.md) | Registers, instruction formats and the full instruction set |
| [Memory.md](./architecture/Memory.md) | Address space, program loading, endianness, alignment and invalid accesses |
| [Interrupts.md](./architecture/Interrupts.md) | The interrupt model and the interrupt codes |
| [Devices.md](./architecture/Devices.md) | The memory-mapped devices and their behaviour |

## Guide

For writing programs and running them.

| Page | Contents |
|:--|:--|
| [First-Program.md](./guide/First-Program.md) | A complete program written from scratch, assembled and run |
| [Assembler.md](./guide/Assembler.md) | Syntax accepted by `tools/asm.py` |
| [Emulator.md](./guide/Emulator.md) | Command line options, exit codes, state snapshots |
| [Programming.md](./guide/Programming.md) | Conventions a bare-machine program must respect |

## Internals

For working on the emulator itself.

| Page | Contents |
|:--|:--|
| [Sequential-Core.md](./internals/Sequential-Core.md) | The sequential core and its execution loop |
| [Device-Model.md](./internals/Device-Model.md) | How devices are implemented, and how to add one |
| [Testing.md](./internals/Testing.md) | The test programs and how to check them |

Circuit schemes and other figures live in [assets/](./assets).
