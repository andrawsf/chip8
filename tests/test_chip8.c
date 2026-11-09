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

/* ------------------------------------------------------------------------
 * Fetch
 *
 * New tests from here down are `static`: if one is written but never called
 * from main(), -Wunused-function + -Werror fails the build.
 * ---------------------------------------------------------------------- */

/* Put two bytes at `pc` and point the PC there. */
static void setup_fetch(uint16_t pc, uint8_t hi, uint8_t lo) {
    machine_init(&machine);
    machine.registers.program_counter = pc;
    machine.memory[pc] = hi;
    machine.memory[pc + 1] = lo;
}

static void test_fetch_combines_bytes_big_endian(void) {
    setup_fetch(0x200, 0x12, 0x34);
    uint16_t opcode = 0;

    chip8_status ret_val = machine_fetch(&machine, &opcode);

    assert(ret_val == CHIP8_OK);
    assert(opcode == 0x1234);
    assert(machine.registers.program_counter == 0x202);
}

/* Swapped bytes give a swapped opcode: catches the high/low bytes being mixed up. */
static void test_fetch_byte_order(void) {
    setup_fetch(0x200, 0x34, 0x12);
    uint16_t opcode = 0;

    machine_fetch(&machine, &opcode);

    assert(opcode == 0x3412);
}

/* All bits set: catches a sign-extension or truncation bug in the shift. */
static void test_fetch_high_bits_survive(void) {
    setup_fetch(0x200, 0xFF, 0xFF);
    uint16_t opcode = 0;

    machine_fetch(&machine, &opcode);

    assert(opcode == 0xFFFF);
}

/* Reads from wherever PC points, not a hard-coded 0x200. */
static void test_fetch_reads_from_current_pc(void) {
    setup_fetch(0x300, 0xAB, 0xCD);
    uint16_t opcode = 0;

    chip8_status ret_val = machine_fetch(&machine, &opcode);

    assert(ret_val == CHIP8_OK);
    assert(opcode == 0xABCD);
    assert(machine.registers.program_counter == 0x302);
}

/* 0xFFE is the last valid PC: reads 0xFFE and 0xFFF. Pins down > vs >=. */
static void test_fetch_at_last_valid_pc(void) {
    setup_fetch(0xFFE, 0x0A, 0x0B);
    uint16_t opcode = 0;

    chip8_status ret_val = machine_fetch(&machine, &opcode);

    assert(ret_val == CHIP8_OK);
    assert(opcode == 0x0A0B);
}

/* On error, neither the output nor the PC may change. 0xBEEF is a sentinel
 * that fetch could never legitimately write here. */
static void assert_fetch_rejected_at(uint16_t pc) {
    machine_init(&machine);
    machine.registers.program_counter = pc;
    uint16_t opcode = 0xBEEF;

    chip8_status ret_val = machine_fetch(&machine, &opcode);

    assert(ret_val == CHIP8_ERR_OUT_OF_BOUNDS);
    assert(opcode == 0xBEEF);
    assert(machine.registers.program_counter == pc);
}

static void test_fetch_below_program_start_rejected(void) {
    assert_fetch_rejected_at(0x1FF);  // one below 0x200
}

static void test_fetch_past_end_of_memory_rejected(void) {
    assert_fetch_rejected_at(0xFFF);  // second byte would be at 0x1000
}

static void test_fetch_null_machine_is_invalid(void) {
    uint16_t opcode = 0;

    assert(machine_fetch(NULL, &opcode) == CHIP8_ERR_INVALID_MACHINE);
}

static void test_fetch_null_out_is_invalid(void) {
    machine_init(&machine);

    assert(machine_fetch(&machine, NULL) == CHIP8_ERR_INVALID_INPUT);
    assert(machine.registers.program_counter == 0x200);
}

/* ------------------------------------------------------------------------
 * Opcode decoding (bitfield union)
 *
 * The union's bit order is compiler-dependent, so these tests are what make
 * relying on it safe. Expected values are written out by hand, and the final
 * test cross-checks every possible opcode against portable masks and shifts.
 * ---------------------------------------------------------------------- */

static void assert_opcode_fields(uint16_t raw, uint8_t type, uint8_t x, uint8_t y,
                                 uint8_t n, uint8_t nn, uint16_t nnn) {
    chip8_opcode opcode = { .raw = raw };

    assert(opcode.nib.type == type);
    assert(opcode.nib.X == x);
    assert(opcode.nib.Y == y);
    assert(opcode.nib.N == n);
    assert(opcode.byte.NN == nn);
    assert(opcode.addr.NNN == nnn);
}

static void test_opcode_decodes_D123(void) {
    assert_opcode_fields(0xD123, 0xD, 0x1, 0x2, 0x3, 0x23, 0x123);
}

/* Every nibble different: catches fields wired to the wrong nibble (e.g. X/Y swapped). */
static void test_opcode_decodes_distinct_nibbles(void) {
    assert_opcode_fields(0xABCD, 0xA, 0xB, 0xC, 0xD, 0xCD, 0xBCD);
}

/* All ones: catches a field that is too narrow (e.g. NNN declared 8 bits wide). */
static void test_opcode_decodes_all_ones(void) {
    assert_opcode_fields(0xFFFF, 0xF, 0xF, 0xF, 0xF, 0xFF, 0xFFF);
}

static void test_opcode_decodes_all_zeros(void) {
    assert_opcode_fields(0x0000, 0x0, 0x0, 0x0, 0x0, 0x00, 0x000);
}

/* Exhaustive: all 65536 opcodes, bitfields vs. masks and shifts. Fast enough
 * to run every build, and it fails immediately on a compiler with a different
 * bitfield layout. */
static void test_opcode_bitfields_match_masks_for_all_values(void) {
    for (uint32_t raw = 0; raw <= 0xFFFF; raw++) {
        chip8_opcode opcode = { .raw = (uint16_t)raw };

        assert(opcode.nib.type == ((raw & 0xF000) >> 12));
        assert(opcode.nib.X    == ((raw & 0x0F00) >> 8));
        assert(opcode.nib.Y    == ((raw & 0x00F0) >> 4));
        assert(opcode.nib.N    ==  (raw & 0x000F));
        assert(opcode.byte.NN  ==  (raw & 0x00FF));
        assert(opcode.addr.NNN ==  (raw & 0x0FFF));
    }
}

/* ------------------------------------------------------------------------
 * Execute
 * ---------------------------------------------------------------------- */

static chip8_status execute_raw(uint16_t raw) {
    chip8_opcode opcode = { .raw = raw };
    return machine_execute(&machine, opcode);
}

/* 6XNN: VX = NN. Other registers untouched (catches writing to VY, or all of V). */
static void test_execute_6XNN_sets_register(void) {
    machine_init(&machine);
    static chip8_machine before;
    memcpy(&before, &machine, sizeof(machine));

    chip8_status ret_val = execute_raw(0x6A42);

    assert(ret_val == CHIP8_OK);
    assert(machine.registers.V[0xA] == 0x42);
    for (int i = 0; i < REGISTER_NUM; i++) {
        if (i != 0xA) {
            assert(machine.registers.V[i] == before.registers.V[i]);
        }
    }
}

/* Execute does not advance the PC itself; that's fetch's job. */
static void test_execute_does_not_advance_pc(void) {
    machine_init(&machine);

    execute_raw(0x6A42);

    assert(machine.registers.program_counter == 0x200);
}

/* 7XNN: VX += NN. */
static void test_execute_7XNN_adds_to_register(void) {
    machine_init(&machine);
    machine.registers.V[3] = 0x10;

    chip8_status ret_val = execute_raw(0x7305);

    assert(ret_val == CHIP8_OK);
    assert(machine.registers.V[3] == 0x15);
}

/* 7XNN wraps at 8 bits and does NOT touch VF (unlike 8XY4, which sets carry).
 * VF is preset to a sentinel so that writing either 0 or 1 to it is caught. */
static void test_execute_7XNN_wraps_without_setting_vf(void) {
    machine_init(&machine);
    machine.registers.V[3] = 0xFF;
    machine.registers.V[0xF] = 0x55;

    execute_raw(0x7301);

    assert(machine.registers.V[3] == 0x00);
    assert(machine.registers.V[0xF] == 0x55);
}

/* ANNN: I = NNN (full 12 bits). */
static void test_execute_ANNN_sets_index(void) {
    machine_init(&machine);

    chip8_status ret_val = execute_raw(0xA123);

    assert(ret_val == CHIP8_OK);
    assert(machine.registers.I == 0x123);
}

/* 1NNN: PC = NNN. */
static void test_execute_1NNN_jumps(void) {
    machine_init(&machine);

    chip8_status ret_val = execute_raw(0x1234);

    assert(ret_val == CHIP8_OK);
    assert(machine.registers.program_counter == 0x234);
}

/* 00E0: every display byte cleared, not just the first. */
static void test_execute_00E0_clears_whole_display(void) {
    machine_init(&machine);
    memset(machine.display, 0xFF, sizeof(machine.display));

    chip8_status ret_val = execute_raw(0x00E0);

    assert(ret_val == CHIP8_OK);
    for (int i = 0; i < DISPLAY_NUM; i++) {
        assert(machine.display[i] == 0);
    }
}

/* Only exactly 00E0 clears the screen. 0FE0 / 01E0 share the low byte but
 * are not valid instructions, and must not wipe the display. */
static void assert_rejected_and_display_kept(uint16_t raw) {
    machine_init(&machine);
    memset(machine.display, 0xFF, sizeof(machine.display));

    chip8_status ret_val = execute_raw(raw);

    assert(ret_val == CHIP8_ERR_UNKNOWN_OPCODE);
    assert(machine.display[0] == 0xFF);
}

static void test_execute_0FE0_is_unknown(void) {
    assert_rejected_and_display_kept(0x0FE0);
}

static void test_execute_01E0_is_unknown(void) {
    assert_rejected_and_display_kept(0x01E0);
}

/* 5XY1 is permanently invalid (5XY0 requires N = 0), so this test stays
 * valid as more opcodes are implemented. An unknown opcode leaves the
 * machine completely unchanged. */
static void test_execute_invalid_opcode_is_unknown(void) {
    machine_init(&machine);
    static chip8_machine before;
    memcpy(&before, &machine, sizeof(machine));

    chip8_status ret_val = execute_raw(0x5001);

    assert(ret_val == CHIP8_ERR_UNKNOWN_OPCODE);
    assert(memcmp(&before, &machine, sizeof(machine)) == 0);
}

static void test_execute_null_machine_is_invalid(void) {
    chip8_opcode opcode = { .raw = 0x6A42 };

    assert(machine_execute(NULL, opcode) == CHIP8_ERR_INVALID_MACHINE);
}

int main(void) {
    // init
    test_init_PC_start();
    test_init_clears_registers();
    test_init_clears_memory();
    test_init_clears_stack();
    test_init_clears_timers();
    test_init_clears_display();
    test_init_clears_keypad();
    test_init_font_digit_0();
    test_init_font_digit_f();
    test_init_null_machine_is_invalid();

    // ROM loading
    test_rom_load_null_rom_is_invalid();
    test_rom_load_null_machine_is_invalid();
    test_rom_load_copies_bytes_to_0x200();
    test_rom_load_max_size_fits();
    test_rom_load_one_byte_too_large();
    test_rom_load_huge_size_not_truncated();

    // fetch
    test_fetch_combines_bytes_big_endian();
    test_fetch_byte_order();
    test_fetch_high_bits_survive();
    test_fetch_reads_from_current_pc();
    test_fetch_at_last_valid_pc();
    test_fetch_below_program_start_rejected();
    test_fetch_past_end_of_memory_rejected();
    test_fetch_null_machine_is_invalid();
    test_fetch_null_out_is_invalid();

    // decode
    test_opcode_decodes_D123();
    test_opcode_decodes_distinct_nibbles();
    test_opcode_decodes_all_ones();
    test_opcode_decodes_all_zeros();
    test_opcode_bitfields_match_masks_for_all_values();

    // execute
    test_execute_6XNN_sets_register();
    test_execute_does_not_advance_pc();
    test_execute_7XNN_adds_to_register();
    test_execute_7XNN_wraps_without_setting_vf();
    test_execute_ANNN_sets_index();
    test_execute_1NNN_jumps();
    test_execute_00E0_clears_whole_display();
    test_execute_0FE0_is_unknown();
    test_execute_01E0_is_unknown();
    test_execute_invalid_opcode_is_unknown();
    test_execute_null_machine_is_invalid();

    printf("All tests passed!\n");
    return 0;
}
