# Running the Emulator

For anyone running programs on tinyDLX and inspecting their results.

## Invocation

```bash
./bin/cpu_seq -b program.bin [options]
```

The binary is loaded in ROM and execution starts at address 0. The program's own output goes to standard output, while the emulator's own messages, marked `[INFO]`, `[WARN]` and `[ERROR]`, go to standard error.

While the emulator runs, the terminal is put in raw mode, so every key pressed reaches the Input Port immediately instead of waiting for a newline. `Ctrl+C` still stops the emulator.

## Options

| Option | Effect |
|:--|:--|
| `-b, --binary-file FILE` | The program to run. Required. |
| `-f, --freq HZ` | Target frequency. The default value is `0`, which makes the emulator run as fast as the host allows. |
| `-C, --max-cycles N` | Stops execution after `N` cycles. The default value is `0`, which means there is no limit. |
| `-G, --init-gpr VALUE` | Initial value of R1 to R31. Without it their content is not deterministic and a program must not rely on it. R0 is always 0. |
| `-d, --dump-state PATH` | Writes a JSON snapshot of the final state to `PATH`. |
| `-s, --strict` | Runs in strict mode, see below. |
| `-h, --help` | Prints the options and exits. |

Numeric values are decimal, hexadecimal with `0x` or octal with a leading `0`; signs are rejected.

## Cycles

The sequential core executes one instruction per cycle, so `--max-cycles`, `--freq` and the `cycles` field of the snapshot all count executed instructions.

A frequency is also the rate at which devices are updated, so it changes how fast a program can drive them.

## Strict mode

Some conditions are errors the machine leaves undefined, described in [Memory.md](../architecture/Memory.md), [ISA.md](../architecture/ISA.md) and [Interrupts.md](../architecture/Interrupts.md). The emulator handles them in one of two ways:

- **normal mode**: a warning is printed on standard error and execution continues. A faulty read returns 0 and a faulty write has no effect.
- **strict mode** (`--strict`): the same condition stops execution with a fault.

The conditions are:

- a half word or word access at a misaligned address;
- a read or a write at an unmapped address, including an operation the addressed device does not support;
- a write to ROM;
- an unknown opcode, or an unknown function code in an R-Type instruction;
- `INT` with a code that is neither `0x80` nor a debug one, and an unknown debug code.

Strict mode is the way to catch a program that relies on undefined behaviour. One case does not depend on it: `INT 0x80` executed while interrupts are disabled always stops the emulator with a fault.

## Exit codes

| Code | Meaning |
|:-:|:--|
| `0` | The program turned the machine off, through the Power Manager or a debug halt. |
| `1` | Execution stopped on a fault. |
| `2` | The emulator received SIGINT or SIGTERM. |
| `3` | Internal emulator error. |
| `4` | The limit set by `--max-cycles` was reached. |

## State snapshot

`--dump-state PATH` writes the final state as JSON, overwriting the file. It holds the cycle count, the reason execution ended (`halt`, `fault`, `signal` or `timeout`), the special registers and all the general purpose ones:

```json
{
  "version": 1,
  "state": {
    "exit_reason": "halt",
    "cycles": 6,
    "pc": "0x00000018",
    "sr": "0x00000001",
    "iar": "0x00000000",
    "cr": "0x00000000",
    "R0": "0x00000000",
    "R1": "0xC0000000",
    "···": "···",
    "R31": "0x00000000"
  }
}
```

Together with `--init-gpr`, which fixes the initial value of the registers, and `--max-cycles`, which stops a run at a known point, the snapshot makes two runs of the same program comparable. RAM is not initialised, so this holds only for programs that write a location before reading it.

## Debug interrupts

Debug interrupts are not part of the machine: the emulator implements them to help the programmer. They use the interrupt codes from `0xF0` up, which the architecture reserves to the development environment. Debug interrupts are handled even if SR[IEN]=0 and do not set any register or flag.

| Code | Name | Description |
|:----|:----|:----|
| **0xF0** | START_TIMER | Starts a timer managed by the emulator. |
| **0xF1** | STOP_TIMER | Stops the emulator timer (started by 0xF0) and outputs the time. |
| **0xF2** | HALT | Explicit debug halt, bypassing the Power Manager. |
