# I/O devices in tinyDLX

Input and output devices are managed as memory-mapped devices (MMIO). This means that the emulated program can interact with a device using store and load instructions at the addresses where it is mapped. The effect of read and write operations differs from device to device.

## Interrupts

Devices can assert an interrupt to the Interrupt Controller (IC), which is a special device used to buffer interrupts. This is necessary because the DLX has only one interrupt line. The trigger mode is level-triggered or edge-triggered depending on the device and not every device needs to assert an interrupt.

When DLX receives an interrupt the PC is saved into IAR and is set to 0. Since both startup and interrupt entry land at address 0, the handler must be able to tell them apart. This can be done with the Startup Circuit device: the code at address 0 must read the Startup Circuit first to determine whether it was entered at reset or on an interrupt.

Interrupts are disabled (SR[IEN]=0) on entry and re-enabled (SR[IEN]=1) by RFE (see [ISA.md](./ISA.md)), so IAR is never overwritten while a handler is running. Nested interrupts are not supported.

## Interrupt Controller

The Interrupt Controller is the only device wired to the DLX interrupt line. In fact, it serves as a buffer for devices' interrupt lines: each device that can assert an interrupt is wired to an interrupt line of the IC. When at least one of its interrupt lines is asserted (by a device) it propagates the interrupt into the DLX's interrupt line. The IC's interrupt output is level-triggered.

Interrupt lines are numbered from 0 to DLX_MAX_DEVICES-1 (0 to 14 currently, since DLX_MAX_DEVICES is 15 and index 15 is reserved). If more than one interrupt line is asserted, the lowest interrupt line index has the highest priority. If no interrupt is asserted, a read operation returns the reserved code `0xF` and so it is never a valid interrupt code.

When the DLX interrupt line is asserted, the handler identifies the source by reading the IC, which returns the index of the highest-priority asserted line. Since the mapping between devices and line indices is fixed, the handler can determine which device generated the interrupt. Reading the IC asserts its CLEAR port, which deasserts that line (see figure below). The IC keeps its output asserted until all pending interrupts have been read.

Note that the deassertion works only inside the IC: the IC does not communicate to the devices that their interrupt has been read. This means that the software must service the device, otherwise it could continue to assert the interrupt if it is level-triggered.

For now only the Input Port can assert an interrupt, so it is the only device wired to the Interrupt Controller (as in figure).

The circuit scheme is as follows.

![Interrupt Controller scheme](../assets/Interrupt_Controller.svg)

*RESET is asynchronous and is asserted on system startup. CS_INTERRUPT_CONTROLLER is from the first-level decoder*

## Startup Circuit

The startup circuit consists of a single DFF which is asserted on emulator startup. A read operation will return the value of the DFF. The DFF value can be set to 0 by a dummy write. On startup the handler at address 0 reads the Startup Circuit, finds it asserted, clears it with a dummy write, and proceeds with initialization. Every later entry at address 0 reads 0 and is therefore an interrupt.

The circuit scheme is as follows.

![Startup Circuit scheme](../assets/Startup_Circuit.svg)

*RESET is asynchronous and is asserted on system startup. CS_STARTUP_CIRCUIT is from the first-level decoder*
