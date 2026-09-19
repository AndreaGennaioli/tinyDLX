# Memory

## Program loading

The program executed by tinyDLX is a **flat binary**: there is no loader, no operating system and no dynamic linking. The program is placed in ROM starting at address `0x00000000` and execution begins at `PC = 0`.

## Endianness

Memory is big-endian: the byte at the lowest address is the most significant one. Half words and words are composed and decomposed accordingly.

## MMIO mappings

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
