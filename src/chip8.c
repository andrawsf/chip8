#include <string.h>
#include <stdio.h>
#include "chip8.h"

#define FONT_START       0x050
#define FONT_CHAR_COUNT  16
#define FONT_CHAR_BYTES  5
#define BITS_PER_BYTE    8

static const uint8_t display_table[FONT_CHAR_COUNT][FONT_CHAR_BYTES] = {
    {0xF0, 0x90, 0x90, 0x90, 0xF0}, // 0
    {0x20, 0x60, 0x20, 0x20, 0x70}, // 1
    {0xF0, 0x10, 0xF0, 0x80, 0xF0}, // 2
    {0xF0, 0x10, 0xF0, 0x10, 0xF0}, // 3
    {0x90, 0x90, 0xF0, 0x10, 0x10}, // 4
    {0xF0, 0x80, 0xF0, 0x10, 0xF0}, // 5
    {0xF0, 0x80, 0xF0, 0x90, 0xF0}, // 6
    {0xF0, 0x10, 0x20, 0x40, 0x40}, // 7
    {0xF0, 0x90, 0xF0, 0x90, 0xF0}, // 8
    {0xF0, 0x90, 0xF0, 0x10, 0xF0}, // 9
    {0xF0, 0x90, 0xF0, 0x90, 0x90}, // A
    {0xE0, 0x90, 0xE0, 0x90, 0xE0}, // B
    {0xF0, 0x80, 0x80, 0x80, 0xF0}, // C
    {0xE0, 0x90, 0x90, 0x90, 0xE0}, // D
    {0xF0, 0x80, 0xF0, 0x80, 0xF0}, // E
    {0xF0, 0x80, 0xF0, 0x80, 0x80}, // F
};

/**
 * Reset the machine to its power-on state: everything zeroed, PC at the
 * start of program memory (0x200), 0x050-0x09F is used for fonts. 
 * Caller owns the storage.
 */
chip8_status machine_init(chip8_machine* machine) {
    if (machine == NULL) {
        return CHIP8_ERR_INVALID_MACHINE;
    }

    // Zero first, then set non-zero fields. If we set the PC first, the
    // memset would wipe it straight back to 0.
    memset(machine, 0, sizeof(*machine));

    machine->registers.program_counter = PROGRAM_START;

    // Set the font memory
    memcpy(&machine->memory[FONT_START], display_table, sizeof(display_table));
    return CHIP8_OK;
}

/**
 * Copy a ROM image into program memory (0x200-0xFFF). See chip8.h.
 */
chip8_status machine_load_rom(chip8_machine* machine, const uint8_t* byte_array, size_t byte_array_size) {
    // Pointer checks first: if a pointer is bad, the size is meaningless.
    if (machine == NULL) {
        return CHIP8_ERR_INVALID_MACHINE;
    }
    if (byte_array == NULL) {
        return CHIP8_ERR_INVALID_INPUT;
    }
    if (byte_array_size > (MEMORY_SIZE - PROGRAM_START)) {
        return CHIP8_ERR_ROM_TOO_LARGE;
    }

    memcpy(&machine->memory[PROGRAM_START], byte_array, byte_array_size);
    return CHIP8_OK;
}

/**
 * Prints out the error status
 */
void machine_display_error(chip8_status status) {
    switch (status) {
        case CHIP8_OK:
            printf("chip8 ok\n");
            break;
        case CHIP8_ERR_INVALID_MACHINE:
            printf("Invalid machine provided\n");
            break;
        case CHIP8_ERR_ROM_TOO_LARGE:
            printf("ROM too large\n");
            break;
        case CHIP8_ERR_INVALID_INPUT:
            printf("Invalid input\n");
            break;
        default:
            printf("not a valid status\n");
    }
}

/**
 * Fetch command which takes the two next areas from memory and then increments
 * the PC
 *
 * Returns unit16_t
 */
chip8_status machine_fetch(chip8_machine* machine, uint16_t* out) {
    if (machine == NULL) {
        return CHIP8_ERR_INVALID_MACHINE;
    }
    if (out == NULL) {
        return CHIP8_ERR_INVALID_INPUT;
    }
    if (machine->registers.program_counter > MEMORY_SIZE - 2 || machine->registers.program_counter < PROGRAM_START) {
        return CHIP8_ERR_OUT_OF_BOUNDS;
    }

    *out = machine->memory[machine->registers.program_counter++] << BITS_PER_BYTE;
    *out |= machine->memory[machine->registers.program_counter++];
    return CHIP8_OK;
}
