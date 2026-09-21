# tinyDLX Overview

tinyDLX is a 32-bit RISC machine derived from the DLX of Hennessy & Patterson. Around its instruction set it defines a complete bare-metal system: a flat memory holding ROM, RAM and memory-mapped devices, and a single interrupt line driven by an interrupt controller.

## Programmer-visible state

- **General purpose registers**: 32 registers of 32 bits, R0..R31. R0 always reads as zero and ignores writes; R31 receives the return address of `JAL` and `JALR`.
- **PC**: the address of the next instruction. Execution starts at `0x00000000`, where the program is loaded.
- **Special registers**: SR, with the interrupt enable flag, IAR and CR, which record where and why the last interrupt happened; see [ISA.md](./ISA.md).
- **Memory**: byte-addressed and big-endian, see [Memory.md](./Memory.md).

## How this section is organised

- [ISA.md](./ISA.md): registers, instruction formats and the full instruction set.
- [Memory.md](./Memory.md): address space, program loading, endianness, alignment and invalid accesses.
- [Interrupts.md](./Interrupts.md): how the machine reacts to hardware and software interrupts.
- [Devices.md](./Devices.md): the memory-mapped devices.

## Differences from DLX

tinyDLX keeps the DLX programming model but is not a complete DLX:

- **No floating point**: no floating point registers, loads, stores, arithmetic or branches.
- **No multiply and divide**, which DLX performs in its floating point unit, and no unsigned register-register `ADDU` and `SUBU`.
- **No access to special registers**: DLX's `MOVI2S` and `MOVS2I` have no counterpart yet.
- **`INT` instead of `TRAP`**: hardware and software interrupts share the entry point at address 0, and the Cause Register records which one occurred.
- **Its own encoding**: the sources that describe DLX do not agree on its opcodes; tinyDLX defines its own in [ISA.md](./ISA.md) and is not binary compatible with other DLX tools.
- **A system around the ISA**: DLX does not define a memory map or devices. The address map, the devices, the interrupt controller and the startup circuit are specific to tinyDLX.
- **Notation**: [ISA.md](./ISA.md) names register fields by their position (RA, RB, RC) rather than by their role (rs1, rs2, rd).
