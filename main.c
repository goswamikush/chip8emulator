#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// RAM
uint8_t chip8ram[4096];

// Registers
// general purpose
uint8_t gp_registers[16];

// special purpose
uint8_t delay_timer;
uint8_t sound_timer;

// pseudo-registers
uint16_t pc;
uint8_t sp;

// Stack
uint16_t stack[16];

// I register
uint16_t I;

// Display
uint8_t display[32][64];

int initialize_values() {
    // RAM from 0 to 4095
    memset(chip8ram, 0, sizeof(chip8ram));

    // General purpose registers
    memset(gp_registers, 0, sizeof(gp_registers));

    // Special purpose registers
    delay_timer = 0;
    sound_timer = 0;

    // Pseudo-registers
    pc = 0x200;
    sp = 0;

    // Stack
    memset(stack, 0, sizeof(stack));

    // I register
    I = 0;

    // Display
    memset(display, 0, sizeof(display));

    return 0;
}

int load_rom(char *filepath) {
    FILE *fptr;

    fptr = fopen(filepath, "rb");

    if (fptr == NULL) {
        printf("Error opening file. Exiting program.\n");
        return 1;
    };

    // Get file size
    fseek(fptr, 0, SEEK_END);

    long file_size = ftell(fptr);

    fseek(fptr, 0, SEEK_SET);

    size_t bytes_read = fread(chip8ram + 0x200, 1, file_size, fptr);

    fclose(fptr);

    return 0;
}

void loop() {
    bool is_running = true;

    while (is_running) {
        // Fetch next instruction
        uint16_t opcode = (chip8ram[pc] << 8) | chip8ram[pc + 1];

        // Increment pc to be ready to fetch next opcode
        pc += 2;

        // Decode instruction into separate nibbles
        uint16_t decoded_inst[4];

        decoded_inst[0] = opcode >> 12;
        decoded_inst[1] = opcode >> 8 & 0xF;
        decoded_inst[2] = opcode >> 4 & 0xF;
        decoded_inst[3] = opcode & 0xF;

        uint16_t second_byte = (decoded_inst[2] << 4) | decoded_inst[3];
        uint16_t last_nibbles = (decoded_inst[1] << 8) | (decoded_inst[2] << 4) | decoded_inst[3];
    };
}
