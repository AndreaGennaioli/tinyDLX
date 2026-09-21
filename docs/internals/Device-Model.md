# Device Model

How the emulator implements the memory-mapped devices described in [Devices.md](../architecture/Devices.md), and how to add a new one.

## The device interface

Every device is a `DLX_device` (`include/dlx_defs.h`):

| Field | Role |
|:--|:--|
| `base_address`, `range_size` | The addresses the device answers to, from `base_address` to `base_address + range_size - 1`. |
| `state` | The device's own data, passed to every handler. |
| `tick` | Called once per cycle. |
| `read`, `write` | Called on a load or a store inside the device's range. |
| `free` | Releases `state` when the emulator exits. |

`tick` and `free` can be `NULL` when there is nothing to do; a `NULL` `read` or `write` means the device does not support that operation.

## Lifecycle

- **Creation**: each device has a constructor, `dlx_<device>_create()`, that allocates the `DLX_device` and its state, and returns `NULL` on failure.
- **Registration**: `setup_devices()` in `src/cpu_seq/main.c` creates every device at its address and registers it. At most `DLX_MAX_DEVICES` devices can be registered, the Interrupt Controller included, and the order of registration is also the order in which devices are ticked.
- **Tick**: at the start of every cycle, before the CPU checks its interrupt line.
- **Access**: the memory bus calls `read` or `write` with the offset from `base_address` and the access width in bytes: 1, 2 or 4. Half word and word accesses reach the device only if aligned. A read must return a value that fits the width, because `LBU` and `LHU` do not mask it. An access the device does not support is treated as an access to an unmapped address.
- **Destruction**: at exit, `free` releases the state, then the `DLX_device` itself is freed.

## Interrupts

A device that asserts interrupts receives the Interrupt Controller as a `DLX_ic_base *` in its constructor and keeps it in its own state, together with its line index. It then calls `assert_interrupt` and `deassert_interrupt` on it. The trigger mode is up to the device: the Input Port is level-triggered, so it asserts its line again at every tick while its byte is unread.

## Adding a device

1. Write `src/devices/dlx_<device>.c` and `include/devices/dlx_<device>.h`, with the state struct and the constructor. The Makefile compiles every file in `src/devices/` on its own.
2. Register it in `setup_devices()`, at an MMIO address that no other device uses: nothing checks for overlapping ranges.
3. If it asserts interrupts, pass it the IC and a free line index. Lower indices have higher priority.
4. Document it in the architecture: its behaviour in [Devices.md](../architecture/Devices.md), its address in the address map of [Memory.md](../architecture/Memory.md) and its interrupt line, if it has one.
5. Add a test program in `tests/`.

`dlx_input_port.c` is the reference for a device that asserts interrupts, `dlx_output_port.c` for one that is polled.
