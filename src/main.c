#include "chip8.h"
#include <stdio.h>

#define READ_SIZE (MEMORY_SIZE - PROGRAM_START + 1)
static chip8_machine machine;
static uint8_t read_buffer[READ_SIZE];

int main(int argc, char *argv[]) {

    chip8_status status = CHIP8_OK;
    if (argc != 2) {
        fprintf(stderr, "no ROM provided\n");
        return 1;
    }

    if (machine_init(&machine) != CHIP8_OK) {
        fprintf(stderr, "machine init failed\n");
        return 1;
    }

    FILE *file_ptr = fopen(argv[1], "rb");
    if(file_ptr == NULL) {
        fprintf(stderr, "file %s failed to open\n", argv[1]);
        return 1;
    }

    size_t bytes_read = fread(read_buffer, sizeof(char), READ_SIZE, file_ptr);
    fclose(file_ptr);

    status |= machine_load_rom(&machine, read_buffer, bytes_read);
    machine_display_error(status);
    return status;
}
