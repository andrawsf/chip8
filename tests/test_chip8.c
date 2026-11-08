#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "chip8.h"

/*
 * One shared machine for all tests. `static` keeps the ~6KB struct off the
 * stack (same habit you'd use on a microcontroller). Every test re-dirties it
 * first, so tests don't depend on each other or on run order.
 */
static chip8_machine machine;

/* Fill every byte with a known non-zero pattern so "it's zero afterwards"
 * actually proves init cleared it. 0xAA = 10101010, easy to spot in a debugger. */
static void make_dirty(chip8_machine* m) {
    memset(m, 0xAA, sizeof(*m));
}

void test_init_PC_start(void) {
    make_dirty(&machine);

    machine_init(&machine);

    // Assert: expected value comes from the CHIP-8 spec, not PROGRAM_START
    assert(machine.registers.program_counter == 0x200);
}

void test_init_clears_registers(void) {
    make_dirty(&machine);

    machine_init(&machine);

    for (int i = 0; i < REGISTER_NUM; i++) {
        assert(machine.registers.V[i] == 0);
    }
    assert(machine.registers.I == 0);
    assert(machine.registers.stack_pointer == 0);
}

void test_init_clears_memory(void) {
    make_dirty(&machine);

    machine_init(&machine);

    for (int i = 0; i < 0x050; i++) {
        assert(machine.memory[i] == 0);
    }
    // skip the font area
    for (int i = 0x100; i < MEMORY_SIZE; i++) {
        assert(machine.memory[i] == 0);
    }
}

void test_init_clears_stack(void) {
    make_dirty(&machine);

    machine_init(&machine);

    for (int i = 0; i < STACK_NUM; i++) {
        assert(machine.stack[i] == 0);
    }
}

void test_init_clears_timers(void) {
    make_dirty(&machine);

    machine_init(&machine);

    assert(machine.delay_timer == 0);
    assert(machine.sound_timer == 0);
}

void test_init_clears_display(void) {
    make_dirty(&machine);

    machine_init(&machine);

    for (int i = 0; i < DISPLAY_NUM; i++) {
        assert(machine.display[i] == 0);
    }
}

void test_init_clears_keypad(void) {
    make_dirty(&machine);

    machine_init(&machine);

    assert(machine.keypad == 0);
}

void test_init_font_digit_0(void) {
    make_dirty(&machine);

    machine_init(&machine);

    assert(machine.memory[0x050] == 0xF0);
    assert(machine.memory[0x051] == 0x90);
    assert(machine.memory[0x052] == 0x90);
    assert(machine.memory[0x053] == 0x90);
    assert(machine.memory[0x054] == 0xF0);
}

void test_init_font_digit_f(void) {
    make_dirty(&machine);

    machine_init(&machine);

    assert(machine.memory[0x050 + 0xF * 5] == 0xF0);
    assert(machine.memory[0x051 + 0xF * 5] == 0x80);
    assert(machine.memory[0x052 + 0xF * 5] == 0xF0);
    assert(machine.memory[0x053 + 0xF * 5] == 0x80);
    assert(machine.memory[0x054 + 0xF * 5] == 0x80);
}

int main(void) {
    test_init_PC_start();
    test_init_clears_registers();
    test_init_clears_memory();
    test_init_clears_stack();
    test_init_clears_timers();
    test_init_clears_display();
    test_init_clears_keypad();
    test_init_font_digit_0();
    test_init_font_digit_f();

    printf("All tests passed!\n");
    return 0;
}
