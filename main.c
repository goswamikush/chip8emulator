#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

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

// Keypad
uint8_t keypad[16];

void display_test();

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

    memset(keypad, 0, sizeof(keypad));

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
    int count = 0;
    while (is_running && count < 50) {
        // Fetch next instruction
        uint16_t opcode = (chip8ram[pc] << 8) | chip8ram[pc + 1];

        // Increment pc to be ready to fetch next opcode
        pc += 2;

        // Decode instruction into separate nibbles
        uint16_t nibbles[4];

        nibbles[0] = opcode >> 12;
        nibbles[1] = opcode >> 8 & 0xF;
        nibbles[2] = opcode >> 4 & 0xF;
        nibbles[3] = opcode & 0xF;

        uint16_t second_byte = (nibbles[2] << 4) | nibbles[3];
        uint16_t last_nibbles = (nibbles[1] << 8) | (nibbles[2] << 4) | nibbles[3];

        if (nibbles[0] == 0 && nibbles[1] == 0) {
            if (second_byte == 0xE0) {
                memset(display, 0, sizeof(display)); 
            };
        };

        // Jump
        if (nibbles[0] == 1) {
            pc = last_nibbles;
        };

        // Set
        if (nibbles[0] == 6) {
            uint16_t gp_index = nibbles[1];

            gp_registers[gp_index] = second_byte;
        };

        // Add
        if (nibbles[0] == 7) {
            uint16_t gp_index = nibbles[1];

            gp_registers[gp_index] += second_byte;
        };

        // Set index
        if (nibbles[0] == 0xA) {
            I = last_nibbles;
        };

        // Display
        if (nibbles[0] == 0xD) {
            uint16_t sprite_height = nibbles[3];

            uint16_t start_x = gp_registers[nibbles[1]] % 64;
            uint16_t start_y = gp_registers[nibbles[2]] % 32;

            bool collision_detected = false;

            int y;
            for (y = 0; y < sprite_height; y++) {
                uint8_t sprite_row = chip8ram[I + y];
                
                int x;
                for (x = 0; x < 8; x++) {
                    uint8_t curr_x = (start_x + x);
                    uint8_t curr_y = (start_y + y);

                    if (curr_x >= 64) {
                        continue;
                    }

                    if (curr_y >= 32) {
                        continue;
                    }

                    uint8_t curr_val = display[curr_y][curr_x];

                    display[curr_y][curr_x] = display[curr_y][curr_x] ^ ((sprite_row >> (7 - x)) & 0x1);

                    uint8_t new_val = display[curr_y][curr_x];

                    if (curr_val == 1 && new_val == 0) {
                        collision_detected = true;
                    }
                }
            };

            if (collision_detected) {
                gp_registers[0xF] = 1;
            } else {
                gp_registers[0xF] = 0;
            };
        };

        // Timers
        if (nibbles[0] == 0xF) {
            uint8_t *reg = &gp_registers[nibbles[1]];

            if (second_byte == 0x07) {
                *reg = delay_timer;
            } else if (second_byte == 0x15) {
                delay_timer = *reg;
            } else if (second_byte == 0x18) {
                sound_timer = *reg;
            };
        };

        // display_test();
        // usleep(100000);
        // count++;
    };
}

void display_test() {
    int y;
    int x;

    for (y = 0; y < 32; y++) {
        for (x = 0; x < 64; x++) {
            uint8_t curr_pixel = display[y][x];

            if (curr_pixel == 1) {
                printf("X");
            } else {
                printf(".");
            };
        };
        printf("\n");
    };
};

int main() {
    initialize_values();
    load_rom("ibm_logo.ch8");
    loop();
    return 0;
}
