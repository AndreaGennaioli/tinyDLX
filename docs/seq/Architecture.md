# Sequential Core Architecture

This implementation of tinyDLX consists in a sequential core with the classic IF-ID-EX-MEM-WB five-stage model.

The program loaded into the emulator is a **flat binary**. This means there is no ELF loader, no OS, no dynamic linking. The program is copied directly into ROM starting at address `0x00000000`, and execution begins at `PC = 0`. This means the program is responsible for interrupt handling, stack management (the is no stack pointer register), and device initialisation.

## Execution model

The sequential emulator runs a synchronous step loop:

```c
while (state.pc < program_size) {
    dlx_seq_step(&state);
}
```

A Power Manager device is planned, which will eventually allow the program to exit the emulator explicitly via a memory mapped write.

Each call to `dlx_seq_step` performs the following operations:
    1. **Tick al devices**: each registered device gets a `tick()` call, allowing it to update internal state and assert interrupt lines id needed.
    2. **Check the interrupt line**: if the interrupt line is asserted and interrupts are enabled (`SR[IEN] == 0`), the CPU saves `PC` into `IAR`, sets `SR[IEN] = 1`, and jumps to address `0x00000000`.
    3. **Fetch**: reads the 32-bit instruction at `PC` from the memory bus. The binary is stored in Big Endian format, no endian conversion is applied at fetch time.
    4. **Decode**: increments `PC` by 4, then decodes the raw instruction.
    5. **Execute/Memory/Write-back**: all collapsed into a single function `execute()`. Memory accesses use the memory bus, which handles address decoding, endian conversion, ad MMIO dispatch.
    6. **Reset the interrupt line**.
    7. **Throttle**: `usleep(···)` is called to avoid saturating the host CPU.
