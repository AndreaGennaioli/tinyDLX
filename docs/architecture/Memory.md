# Memory

## Address space

The address space is 32 bits wide and is divided into three regions:

- **ROM**, 64 KB from `0x00000000`: it holds the program and is read-only.
- **RAM**, 2 MB from `0x40000000`: its content at reset is undefined, so a program must write a location before reading it.
- **MMIO**, from `0xC0000000` to `0xFFFFFFFF`: the memory-mapped devices, each at its own address.

Any other address, including an MMIO address not assigned to a device, is unmapped, and accessing it is an [invalid access](#invalid-access). The table at the end of this page lists every mapped range.

## Program loading

The program executed by tinyDLX is a **flat binary**: there is no loader, no operating system and no dynamic linking. The program is placed in ROM starting at address `0x00000000` and execution begins at `PC = 0`.

## Endianness

Memory is big-endian: the byte at the lowest address is the most significant one. Half words and words are composed and decomposed accordingly.

## Alignment

A half word access must be at an even address, and a word access at an address multiple of 4. Byte accesses have no constraint. An unaligned access is invalid.

## Invalid access

The following accesses are invalid:

- a read or a write at an address that is not mapped;
- an operation the addressed device does not support, such as a write to the Input Port or a read from the Power Manager;
- a write to ROM;
- an unaligned access.

Their effect is undefined: the architecture guarantees neither the value returned by a read, nor the state of memory and devices after a write, nor that execution continues. No exception is defined for them, so a program can neither detect an invalid access nor recover from one: a correct program never performs any.

## Address map

| Device name | Memory Address/Range | Read action | Write action |
|:------------|:---------------|:------------|:-------------|
| **ROM** | 0x00000000:0x0000FFFF (64 KB) | Read data at specified address | - |
| **RAM** | 0x40000000:0x401FFFFF (2 MB) | Read data at specified address | Write data at specified address |
| **Startup Circuit** | 0xC0000000 | Read Startup FF-D value | Set Startup FF-D value to 0 |
| **Input Port** | 0xC0040000 | Read the byte held by the input latch | - |
| **Output Port** | 0xC0080000 | Read port status (0 = busy, 1 = ready) | Send a byte to the external output unit |
| **Interrupt Controller** | 0xC00C0000 | Read code of the active interrupt | - |
| **Power Manager** | 0xC0100000 | - | Turn the system off |

Every device is described in [Devices.md](./Devices.md).
