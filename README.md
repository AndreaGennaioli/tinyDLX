# tinyDLX - DLX Processor Emulator

A didactic emulator for the DLX architecture (Hennessy & Patterson) written in C.
This project aims to develop two implementations:
1. **Sequential DLX**: a standard IF-ID-EX-MEM-WB serial execution (CPI = 5).
2. **Pipelined DLX**: a pipelined implementation (CPI = 1) with hazard handling.

## Project state
**WORK IN PROGRESS**
Next steps: define global structures in `include/`, like the **ISA definitions** or **System parameters**.

## Project structure
- `bin/`: compiled files.
- `docs/`: ISA reference (and more, maybe).
- `include/`: shared header files (eg: ISA definitions, System parameters, common structures).
- `obj/`: object files.
- `src/`: source code.
- `tests/`: assembly scripts.
- `tools/`: useful tools like an assembler.

## Build
```bash
make
```

