#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define MEMORY_SIZE 0X1000
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define DISPLAY_NUM ((DISPLAY_WIDTH * DISPLAY_HEIGHT) / 8)
#define REGISTER_NUM 16
#define STACK_NUM 16

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
 * Initialize machine and set PC.
 *
 * The Chip-8 language is capable of accessing up to 4KB (4,096 bytes) of RAM, 
 * from location 0x000 (0) to 0xFFF (4095). Most Chip-8 programs start at location 0x200
 */
void machine_init(chip8_machine* machine);

#endif //CHIP8_H