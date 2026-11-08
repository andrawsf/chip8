#include "chip8.h"

static chip8_machine machine;

int main(void) {
    machine_init(&machine);
    return 0;
}
