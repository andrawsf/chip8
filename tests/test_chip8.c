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
    // skip the font area 0x050-0x09F
    for (int i = 0x0A0; i < MEMORY_SIZE; i++) {
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

void test_rom_load_null_rom_is_invalid(void) {
    // Arrange: a freshly initialised machine, plus a snapshot to compare against
    machine_init(&machine);
    static chip8_machine before;
    memcpy(&before, &machine, sizeof(machine));

    chip8_status ret_val = machine_load_rom(&machine, NULL, 10);

    // Assert: right error code AND the machine was not touched.
    // Checking the error alone wouldn't catch a loader that half-wrote memory
    // before noticing the problem.
    assert(ret_val == CHIP8_ERR_INVALID_INPUT);
    assert(memcmp(&before, &machine, sizeof(machine)) == 0);
}

void test_rom_load_null_machine_is_invalid(void) {
    uint8_t rom[] = {0x12, 0x34};

    chip8_status ret_val = machine_load_rom(NULL, rom, sizeof(rom));

    assert(ret_val == CHIP8_ERR_INVALID_MACHINE);
}

void test_init_null_machine_is_invalid(void) {
    chip8_status ret_val = machine_init(NULL);

    assert(ret_val == CHIP8_ERR_INVALID_MACHINE);
}

void test_rom_load_copies_bytes_to_0x200(void) {
    machine_init(&machine);
    const uint8_t rom[] = {0x12, 0x34, 0x56};

    chip8_status ret_val = machine_load_rom(&machine, rom, sizeof(rom));

    assert(ret_val == CHIP8_OK);
    assert(machine.memory[0x200] == 0x12);
    assert(machine.memory[0x201] == 0x34);
    assert(machine.memory[0x202] == 0x56);
    // Byte right after the ROM is untouched (catches copying too much)
    assert(machine.memory[0x203] == 0x00);
}

/* Program memory is 0x200-0xFFF = 0xE00 bytes. Test exactly at the limit
 * and one past it to pin down the > vs >= in the size check. */
static uint8_t big_rom[0xE00 + 1];

void test_rom_load_max_size_fits(void) {
    machine_init(&machine);
    memset(big_rom, 0xAB, sizeof(big_rom));

    chip8_status ret_val = machine_load_rom(&machine, big_rom, 0xE00);

    assert(ret_val == CHIP8_OK);
    assert(machine.memory[0x200] == 0xAB);
    assert(machine.memory[0xFFF] == 0xAB); // last byte of memory
}

void test_rom_load_one_byte_too_large(void) {
    machine_init(&machine);
    static chip8_machine before;
    memcpy(&before, &machine, sizeof(machine));

    chip8_status ret_val = machine_load_rom(&machine, big_rom, 0xE00 + 1);

    assert(ret_val == CHIP8_ERR_ROM_TOO_LARGE);
    assert(memcmp(&before, &machine, sizeof(machine)) == 0);
}

/* 66000 truncates to 464 in a uint16_t, so this only passes if the size
 * parameter is wide enough (size_t). Regression test for that bug. */
void test_rom_load_huge_size_not_truncated(void) {
    machine_init(&machine);

    chip8_status ret_val = machine_load_rom(&machine, big_rom, 66000);

    assert(ret_val == CHIP8_ERR_ROM_TOO_LARGE);
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
    test_rom_load_null_rom_is_invalid();
    test_rom_load_null_machine_is_invalid();
    test_init_null_machine_is_invalid();
    test_rom_load_copies_bytes_to_0x200();
    test_rom_load_max_size_fits();
    test_rom_load_one_byte_too_large();
    test_rom_load_huge_size_not_truncated();

    printf("All tests passed!\n");
    return 0;
}
