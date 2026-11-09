# CHIP-8 Interpreter (C)

A CHIP-8 interpreter written in C, built as practice for embedded C development.

## What is CHIP-8?

CHIP-8 is a simple interpreted programming language from the mid-1970s. It was originally created
by Joseph Weisbecker for the COSMAC VIP and Telmac 1800 microcomputers so that games could be written
once and run on different hardware. Today it's a classic first emulator project: the system is small
enough to fully understand, yet it covers the same concepts as real hardware emulation (fetch/decode/execute,
memory maps, registers, timers, and I/O).

### The machine at a glance

| Component        | Details                                                                 |
|------------------|-------------------------------------------------------------------------|
| Memory           | 4 KB (4096 bytes). `0x000–0x1FF` reserved (font data lives here), programs load at `0x200` |
| Registers        | 16 general-purpose 8-bit registers `V0`–`VF` (`VF` doubles as a flag register) |
| Index register   | `I`, 16-bit, used for memory addresses                                  |
| Program counter  | `PC`, 16-bit, points at the current instruction                         |
| Stack            | 16 levels of 16-bit return addresses, with a stack pointer              |
| Timers           | Delay timer and sound timer, both 8-bit, count down at 60 Hz            |
| Display          | 64 × 32 monochrome pixels; sprites are drawn with XOR                   |
| Input            | 16-key hex keypad (`0`–`F`)                                             |
| Instructions     | 35 opcodes, each 2 bytes long and stored big-endian                     |

### Why it's good embedded C practice

- Fixed-width types (`uint8_t`, `uint16_t`) and careful handling of overflow
- Bit manipulation: masking and shifting to decode opcodes (`(op & 0x0F00) >> 8`)
- Endianness: building a 16-bit opcode from two bytes
- Working with a fixed memory map and bounds checking
- Modelling hardware state in a struct
- Timing: running the CPU and the 60 Hz timers at different rates
- A state machine (the fetch → decode → execute loop)

## Roadmap

- [X] **1. Project setup**: git repo, `.gitignore`, `Makefile`, compiler warnings (`-Wall -Wextra -Werror`)
- [X] **2. Machine state**: define a struct holding memory, registers, `I`, `PC`, stack, SP, timers, display, and keypad
- [X] **3. Initialization**: zero the state, set `PC = 0x200`, load the built-in font into low memory
- [X] **4. ROM loading**: read a ROM file into memory starting at `0x200`, with size checks
- [X] **5. Fetch**: read two bytes at `PC`, combine them into a 16-bit opcode, then advance `PC`
- [ ] **6. Decode & execute**: switch on the top nibble and extract `X`, `Y`, `N`, `NN`, and `NNN` with masks
- [ ] **7. Core opcodes first**: `00E0` (clear), `1NNN` (jump), `6XNN` (set), `7XNN` (add), `ANNN` (set I), `DXYN` (draw). These are enough to run the IBM logo ROM
- [ ] **8. Display output**: render the 64×32 framebuffer (terminal first, SDL optional)
- [ ] **9. Remaining opcodes**: subroutines, conditionals, arithmetic and logic (`8XYN`), timers, memory ops, and keypad
- [ ] **10. Timers**: decrement the delay and sound timers at 60 Hz, independent of CPU speed
- [ ] **11. Input**: map keyboard keys to the 16-key hex keypad
- [ ] **12. Testing**: run test ROMs (e.g. the Timendus CHIP-8 test suite) and fix the quirks they reveal
- [ ] **13. Polish**: clean up the code, handle errors, and write up what I learned

## Build & Run

_To be filled in once the Makefile exists._

## References

- [Cowgod's CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [Tobias V. Langhoff, *Guide to making a CHIP-8 emulator*](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)
- [Timendus CHIP-8 test suite](https://github.com/Timendus/chip8-test-suite)
