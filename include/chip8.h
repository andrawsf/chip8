#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define MEMORY_SIZE 0X1000
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define DISPLAY_NUM ((DISPLAY_WIDTH * DISPLAY_HEIGHT) / 8)
#define REGISTER_NUM 16
#define STACK_NUM 16

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

_Static_assert(sizeof(((chip8_machine *)0)->memory) == 0X1000 , "memory size failure");

#endif //CHIP8_H