# Memory mappings

| Device name | Memory Address/Range | Read action | Write action |
|:------------|:---------------|:------------|:-------------|
| **ROM** | 0x00000000:0x0000FFFF (64 KB) | Read data at specified address | - |
| **RAM** | 0x40000000:0x401FFFFF (2 MB) | Read data at specified address | Write data at specified address |
| **Startup Circuit** | 0xC0000000 | Read Startup FF-D value | Set Startup FF-D value to 0 |
| **Input Port** | 0xC0040000 | Read byte from stdin buffer | - |
| **Output Port** | 0xC0080000 | Read port status (0 = busy, 1 = ready) | Write byte to stdout buffer |
| **Interrupt Controller** | 0xC00C0000 | Read code of the active interrupt | - |
| **Power Manager** | 0xC0100000 | - | Exit emulator |
