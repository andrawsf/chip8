#include "chip8.h"

static chip8_machine machine;

int main(void) {
    if (machine_init(&machine) != CHIP8_OK) {
        return 1;
    }
    return 0;
}
