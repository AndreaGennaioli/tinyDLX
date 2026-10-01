# I/O devices in tinyDLX

Input and output devices are managed as memory-mapped devices (MMIO). This means that a program can interact with a device using store and load instructions at the addresses where it is mapped. The effect of read and write operations differs from device to device. The address of every device is listed in [Memory.md](./Memory.md), and the way devices signal the CPU is described in [Interrupts.md](./Interrupts.md).

For now each device is an 8 bit register at its base address and must be accessed with byte loads and stores. It drives data on BD[24..31], accordingly with the big-endian memory and bus layout described in [Memory.md](./Memory.md).

## Interrupt Controller

The Interrupt Controller is the only device wired to the CPU's interrupt line. In fact, it serves as a buffer for devices' interrupt lines: each device that can assert an interrupt is wired to an interrupt line of the IC. When at least one of its interrupt lines is asserted (by a device) it propagates the interrupt into the CPU's interrupt line. The IC's interrupt output is level-triggered.

Interrupt lines are numbered from 0 to 14. If more than one is asserted, the lowest index has the highest priority. If none is asserted, a read returns the reserved code `0xF`, which is never a valid line index.

When the CPU's interrupt line is asserted, the handler identifies the source by reading the IC, which returns the index of the highest-priority asserted line. Since the mapping between devices and line indices is fixed, the handler can determine which device generated the interrupt. Reading the IC asserts its CLEAR port, which deasserts that line (see figure below). The IC keeps its output asserted until all pending interrupts have been read.

Note that the deassertion works only inside the IC: the IC does not communicate to the devices that their interrupt has been read. This means that the software must service the device, otherwise it could continue to assert the interrupt if it is level-triggered.

The circuit scheme is as follows.

![Interrupt Controller scheme](../assets/Interrupt_Controller.svg)

*RESET is asynchronous and is asserted on system startup. CS_INTERRUPT_CONTROLLER is from the first-level decoder*

## Startup Circuit

The Startup Circuit consists of a single DFF, which is set at reset. A read operation will return the value of the DFF. The DFF value can be set to 0 by a dummy write. On startup the handler at address 0 reads the Startup Circuit, finds it asserted, clears it with a dummy write, and proceeds with initialization. Every later entry at address 0 reads 0 and is therefore an interrupt.

The circuit scheme is as follows.

![Startup Circuit scheme](../assets/Startup_Circuit.svg)

*RESET is asynchronous and is asserted on system startup. CS_STARTUP_CIRCUIT is from the first-level decoder*

## Input Port

The Input Port is the device used to standardize external input units. It exposes one byte of input at a time and is wired to interrupt line 0 of the IC.

The port holds a single byte latch. When the external unit presents a byte and the latch is free, the byte is latched and the port asserts its interrupt line. The port does not buffer more than one byte: while the latch is full no further byte is accepted, and the external unit has to hold the next one until the port is free again.

The port's interrupt output is level-triggered and stays asserted for as long as the latch holds an unread byte. Reading the IC clears the line inside the IC, but the Input Port keeps reasserting it until the port itself is read, so the handler must always service the port and not just acknowledge the IC.

A read returns the latched byte, empties the latch and deasserts the interrupt line, which frees the port to latch the next byte. A read performed when no byte is pending returns the last byte that was latched, so software must not use the data value alone to decide whether input has arrived. The port has no write port.

## Output Port

The Output Port is the device used to standardize external output units. It accepts one byte at a time and exposes a status flag; it has no interrupt line, so it is driven by polling only.

A read returns the port status: `1` when the port is ready to accept a byte, `0` while it is busy transferring the previous one.

A write sends the least significant byte of the written value to the external unit; the remaining bits are ignored. The port then goes busy for a fixed number of clock cycles, during which the status reads `0`. A write performed while the port is busy is discarded: the byte is lost and no error is signalled. Software must therefore read the status and wait for it to be `1` before every write, and must not rely on a fixed number of cycles between two writes.

## Power Manager

The Power Manager is the device used to turn off the system. It has neither an interrupt line nor a read port: the only supported operation is a write.

A write turns the system off, regardless of the value written. The shutdown is a normal one: the system stops after the instruction that performed the write, and no further instruction is executed.

## Timer

The Timer is a programmable 32 bit timer that asserts an interrupt each `P` clock cycles. It consists of 3 parts: a 32 bit period register used to store the `P` value; a 32 bit counter; a single DFF used to store its internal status (0 = idle, 1 = interrupt asserted). Its interrupt line in the IC is 1.

On reset `P` is set to 0, which means the timer is disabled. If `P > 0` the counter starts incrementing its value. Once the counter value is equal to the period (`P`) it is set to zero, the status DFF is set to 1 and the interrupt line is asserted. The counter continues to increment. The interrupt is level-triggered until an ack is received. The ack consists of a read on the status register, which returns its value, clears it and deasserts the interrupt.

The period register is writable and readable only by a word access operation on the first 4 bytes of the device address range, any other access size is an invalid access. An overwrite of the period sets the counter to 0, the status to 0 (idle) and deasserts the interrupt line. The status register can be read by a word read at offset 0x4 or a half-word read at offset 0x6 or a single byte read at offset 0x7; any other access to the second 4 bytes, writes included, is an invalid access.

The handler must read the IC before acknowledging the Timer. The ack also deasserts the Timer's line inside the IC, so if the status is read first the IC no longer reports line 1 and returns the next pending line, or `0xF` if there is none.

Given `H` as the execution clocks of the timer interrupt's handler, it is very important that `H << P`. The counter keeps counting while the handler runs, so only `P - H` cycles out of every `P` are left to the rest of the program. If `H >= P` a new interrupt is already pending when the handler returns with `RFE`: the CPU enters the handler again immediately and the rest of the program makes no progress. Moreover the status DFF cannot count expirations, so an expiration that occurs before the previous one has been acknowledged is lost.
