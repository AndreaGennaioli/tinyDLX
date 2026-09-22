# Sequential Core

tinyDLX is emulated by a sequential core that executes one instruction per cycle. The classic IF-ID-EX-MEM-WB stages are logical steps performed in order within a single call to `dlx_seq_step`, and every instruction is completed before the next one is fetched. As a consequence, the `cycles` counter, the `--max-cycles` limit and the `--freq` target all count executed instructions.

How a program is loaded and how memory is addressed is a property of the machine: see [Memory.md](../architecture/Memory.md).

## Source layout

The source code is distributed between 3 directories, with the matching headers under `include/`:

- `src/cpu_seq/`: the sequential core, which decodes and executes every instruction, and the emulator's entry point.
- `src/common/`: the modules the core builds on: machine state, memory bus, program loader, command line, snapshots.
- `src/devices/`: the memory-mapped devices, one file per device.

## Execution model

The core exposes a single function, `dlx_seq_step()` (`src/cpu_seq/dlx_seq_core.c`), which advances the machine by one cycle. The driver around it is the main loop of `src/cpu_seq/main.c`:

```c
while (state.exec_state == DLX_RUNNING) {
    dlx_seq_step(&state);
    state.cycles++;
    /* ... cycle limit and frequency synchronisation ... */
}
```

The loop runs until something changes `exec_state`: a write to the Power Manager or a debug halt (`DLX_HALT`), a fault (`DLX_FAULT`), SIGINT or SIGTERM (`DLX_SIGNAL`), or the limit set by `--max-cycles` (`DLX_TIMEOUT`). There is no implicit exit at the end of the program: ROM is zero-filled, so a program that runs past its last instruction keeps fetching words that decode to no instruction.

Each call to `dlx_seq_step` performs the following operations:

1. **Tick all devices**: each registered device gets a `tick()` call, allowing it to update internal state and assert interrupt lines if needed.
2. **Check the interrupt line**: if the interrupt line is asserted and interrupts are enabled (`SR[IEN] == 1`), the CPU saves `PC` into `IAR`, sets `SR[IEN] = 0`, sets `CR` to 0 and jumps to address `0x00000000`.
3. **Fetch**: reads the 32-bit instruction at `PC` from the memory bus.
4. **Decode**: increments `PC` by 4, then decodes the raw instruction.
5. **Execute/Memory/Write-back**: all collapsed into a single function `execute()`. Memory accesses use the memory bus, which handles address decoding, endianness conversion and MMIO dispatch.
6. **Reset the interrupt line**: it will be asserted again by the Interrupt Controller on the next tick if a request is still pending.

## Frequency synchronisation

Throttling is implemented in the main loop. When `--freq` is zero the emulator runs as fast as the host allows. When a frequency is requested, every `sync_interval` cycles (about a hundred times per second of simulated time) the loop compares the time the program should have taken, `cycles / freq_hz`, with the time actually elapsed, and sleeps for the difference when it is running ahead. If it is running behind, no correction is possible and the emulator simply proceeds at full speed.
