#include <string.h>
#include "chip8.h"

#define PROGRAM_START 0x200

/**
 * Reset the machine to its power-on state: everything zeroed, PC at the
 * start of program memory (0x200). Caller owns the storage.
 */
void machine_init(chip8_machine* machine) {
    if (machine == NULL) {
        return;
    }

    // Zero first, then set non-zero fields. If we set the PC first, the
    // memset would wipe it straight back to 0.
    memset(machine, 0, sizeof(*machine));

    machine->registers.program_counter = PROGRAM_START;
}
