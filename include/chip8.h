#ifndef CHIP8_H
#define CHIP8_H

#include <stddef.h>
#include <stdint.h>

#define MEMORY_SIZE 0X1000
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define DISPLAY_NUM ((DISPLAY_WIDTH * DISPLAY_HEIGHT) / 8)
#define REGISTER_NUM 16
#define STACK_NUM 16

typedef enum {
    CHIP8_OK,
    CHIP8_ERR_INVALID_MACHINE,
    CHIP8_ERR_ROM_TOO_LARGE,
    CHIP8_ERR_INVALID_INPUT
} chip8_status;

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

#endif //CHIP8_H