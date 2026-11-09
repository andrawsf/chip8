#ifndef CHIP8_H
#define CHIP8_H

#include <stddef.h>
#include <stdint.h>

#define MEMORY_SIZE 0X1000
#define PROGRAM_START 0x200

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define DISPLAY_NUM ((DISPLAY_WIDTH * DISPLAY_HEIGHT) / 8)
#define REGISTER_NUM 16
#define STACK_NUM 16

typedef enum {
    CHIP8_OK,
    CHIP8_ERR_INVALID_MACHINE,
    CHIP8_ERR_ROM_TOO_LARGE,
    CHIP8_ERR_INVALID_INPUT,
    CHIP8_ERR_OUT_OF_BOUNDS,
    CHIP8_ERR_UNKNOWN_OPCODE
} chip8_status;

/* OPCODE LAYOUT
 * Every CHIP-8 instruction is 16 bits = 4 nibbles (hex digits), named:
 *
 * opcode 0xD123:
 * 
 *   D      1      2      3
 *   ↑      ↑      ↑      ↑
 *  type    X      Y      N
 *               └─NN─┘
 *        └────NNN────┘
 * 
 * ┌───────┬──────────────┬────────────────────────────┬────────────────┐
 * │ Field │ Bits         │ Meaning                    │ Example 0xD123 │
 * ├───────┼──────────────┼────────────────────────────┼────────────────┤
 * │ type  │ top nibble   │ which instruction group    │ 0xD            │
 * ├───────┼──────────────┼────────────────────────────┼────────────────┤
 * │ X     │ 2nd nibble   │ a register index, VX       │ 0x1            │
 * ├───────┼──────────────┼────────────────────────────┼────────────────┤
 * │ Y     │ 3rd nibble   │ another register index, VY │ 0x2            │
 * ├───────┼──────────────┼────────────────────────────┼────────────────┤
 * │ N     │ last nibble  │ a 4-bit number             │ 0x3            │
 * ├───────┼──────────────┼────────────────────────────┼────────────────┤
 * │ NN    │ last byte    │ an 8-bit number            │ 0x23           │
 * ├───────┼──────────────┼────────────────────────────┼────────────────┤
 * │ NNN   │ last 12 bits │ an address                 │ 0x123          │
 * └───────┴──────────────┴────────────────────────────┴────────────────┘
 *
 * PORTABILITY: bitfield layout is implementation-defined. This union assumes
 * the compiler allocates bitfields starting from the least significant bit
 * (GCC/Clang on little-endian targets), so N is the lowest nibble and type the
 * highest. Write the opcode to `raw`, then read whichever view you need.
 * The size is checked below at compile time; the bit order is checked by the
 * test_opcode_* unit tests, which compare against portable mask/shift results.
 */
typedef union {
    uint16_t raw;
    struct {
        uint16_t N       : 4; // last nibble
        uint16_t Y       : 4; // 3rd nibble
        uint16_t X       : 4; // 2nd nibble
        uint16_t type    : 4; // 1st nibble
    } nib;
    struct {
        uint16_t NN      : 8; // low byte
        uint16_t unused  : 8; // unused
    } byte;
    struct {
        uint16_t NNN     : 12;// low 12 bits
        uint16_t unused  : 4; // unused
    } addr;
} chip8_opcode;

_Static_assert(sizeof(chip8_opcode) == 2, "chip8_opcode must pack into 16 bits");

/**
 * MACHINE MEMORY LAYOUT
 */

typedef struct {
    uint8_t V[REGISTER_NUM]; // General-purpose registers 
    uint16_t I; // Holds a memory address
    uint16_t program_counter; // the address of the next instruction
    uint8_t stack_pointer; // which stack slot is next
} chip8_registers;

typedef struct {
    uint8_t memory[MEMORY_SIZE];
    chip8_registers registers;
    uint16_t stack[STACK_NUM];
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t display[DISPLAY_NUM];
    uint16_t keypad;
} chip8_machine;

/**
 * Reset the machine to its power-on state: everything zeroed, font loaded
 * at 0x050-0x09F, PC set to 0x200. Caller owns the storage.
 *
 * Returns CHIP8_OK, or CHIP8_ERR_INVALID_MACHINE if machine is NULL.
 */
chip8_status machine_init(chip8_machine* machine);

/**
 * Copy a ROM image into program memory starting at 0x200. Program memory
 * runs 0x200-0xFFF, so the largest ROM is 0xE00 (3584) bytes. On any error
 * the machine is left unchanged.
 *
 * Returns CHIP8_OK, or:
 *   CHIP8_ERR_INVALID_MACHINE  machine is NULL
 *   CHIP8_ERR_INVALID_INPUT    byte_array is NULL
 *   CHIP8_ERR_ROM_TOO_LARGE    byte_array_size > 3584
 */
chip8_status machine_load_rom(chip8_machine* machine, const uint8_t* byte_array, size_t byte_array_size);

/**
 * Prints out the error status
 */
void machine_display_error(chip8_status status);

/**
 * Read the next instruction: the two bytes at PC, combined big-endian
 * (memory[PC] is the high byte), into *out. Then advance PC by 2.
 * Valid PC range is 0x200-0xFFE (both bytes must lie inside memory).
 * On any error, *out and PC are left unchanged.
 *
 * Returns CHIP8_OK, or:
 *   CHIP8_ERR_INVALID_MACHINE  machine is NULL
 *   CHIP8_ERR_INVALID_INPUT    out is NULL
 *   CHIP8_ERR_OUT_OF_BOUNDS    PC < 0x200 or PC > 0xFFE
 */
chip8_status machine_fetch(chip8_machine* machine, uint16_t* out);

/**
 * Execute one already-fetched instruction. Does not fetch or advance PC
 * itself (jumps set PC directly).
 *
 * Implemented: 00E0 (clear screen), 1NNN (jump), 6XNN (VX = NN),
 *              7XNN (VX += NN, wraps, VF unchanged), ANNN (I = NNN)
 *
 * Returns CHIP8_OK, or:
 *   CHIP8_ERR_INVALID_MACHINE  machine is NULL
 *   CHIP8_ERR_UNKNOWN_OPCODE   opcode not implemented / not a valid instruction
 */
chip8_status machine_execute(chip8_machine* machine, chip8_opcode opcode);

#endif //CHIP8_H
