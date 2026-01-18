#include "chip8.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Font
static const uint8_t font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8_init(Chip8 *chip8) {
    memset(chip8->memory, 0, sizeof(chip8->memory));
    memset(chip8->V, 0, sizeof(chip8->V));
    memset(chip8->stack, 0, sizeof(chip8->stack));
    memset(chip8->display, 0, sizeof(chip8->display));
    memset(chip8->keypad, 0, sizeof(chip8->keypad));

    chip8->delay_timer = 0;
    chip8->sound_timer = 0;
    chip8->pc = PROGRAM_START;
    chip8->sp = 0;
    chip8->I = 0;

    // Load font into memory
    memcpy(chip8->memory + FONT_START_ADDRESS, font, 80);
}

bool chip8_load_rom(Chip8 *chip8, const char *filepath) {
    FILE *fptr = fopen(filepath, "rb");

    if (fptr == NULL) {
        printf("Error opening file. Exiting program.\n");
        return false;
    }

    fseek(fptr, 0, SEEK_END);
    long file_size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    size_t bytes_read = fread(chip8->memory + PROGRAM_START, 1, file_size, fptr);
    fclose(fptr);

    return true;
}

void chip8_update_timers(Chip8 *chip8) {
    if (chip8->delay_timer > 0) {
        chip8->delay_timer--;
    }

    if (chip8->sound_timer > 0) {
        chip8->sound_timer--;
    }
}

void chip8_execute_cycle(Chip8 *chip8) {
    // Fetch next instruction
    uint16_t opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc + 1];

    // Increment pc to be ready to fetch next opcode
    chip8->pc += 2;

    // Decode instruction into separate nibbles
    uint16_t nibbles[4];
    nibbles[0] = opcode >> 12;
    nibbles[1] = (opcode >> 8) & 0xF;
    nibbles[2] = (opcode >> 4) & 0xF;
    nibbles[3] = opcode & 0xF;

    uint16_t second_byte = (nibbles[2] << 4) | nibbles[3];
    uint16_t last_nibbles = (nibbles[1] << 8) | (nibbles[2] << 4) | nibbles[3];

    // Execute instruction
    switch (nibbles[0]) {
        case 0x0:
            // Clear screen and return from subroutine
            switch (second_byte) {
                case 0xE0:
                    // Clear screen
                    memset(chip8->display, 0, sizeof(chip8->display));
                    break;
                case 0xEE:
                    // Return from subroutine
                    chip8->sp--;
                    chip8->pc = chip8->stack[chip8->sp];
                    break;
            }
            break;

        case 0x1:
            // Jump to address NNN
            chip8->pc = last_nibbles;
            break;

        case 0x2:
            // Call subroutine at NNN
            chip8->stack[chip8->sp] = chip8->pc;
            chip8->sp++;
            chip8->pc = last_nibbles;
            break;

        case 0x3:
            // Skip next instruction if VX == NN
            if (chip8->V[nibbles[1]] == second_byte) {
                chip8->pc += 2;
            }
            break;

        case 0x4:
            // Skip next instruction if VX != NN
            if (chip8->V[nibbles[1]] != second_byte) {
                chip8->pc += 2;
            }
            break;

        case 0x5:
            // Skip next instruction if VX == VY
            if (nibbles[3] == 0) {
                if (chip8->V[nibbles[1]] == chip8->V[nibbles[2]]) {
                    chip8->pc += 2;
                }
            }
            break;

        case 0x6:
            // Set VX to NN
            chip8->V[nibbles[1]] = second_byte;
            break;

        case 0x7:
            // Add NN to VX
            chip8->V[nibbles[1]] += second_byte;
            break;

        case 0x8: {
            // Arithmetic and logic operations
            uint8_t x = nibbles[1];
            uint8_t y = nibbles[2];

            switch (nibbles[3]) {
                case 0x0:
                    // Set VX to VY
                    chip8->V[x] = chip8->V[y];
                    break;
                case 0x1:
                    // Set VX to VX OR VY
                    chip8->V[x] = chip8->V[x] | chip8->V[y];
                    break;
                case 0x2:
                    // Set VX to VX AND VY
                    chip8->V[x] = chip8->V[x] & chip8->V[y];
                    break;
                case 0x3:
                    // Set VX to VX XOR VY
                    chip8->V[x] = chip8->V[x] ^ chip8->V[y];
                    break;
                case 0x4: {
                    // Add VY to VX, set VF to carry
                    uint16_t result = chip8->V[x] + chip8->V[y];
                    chip8->V[0xF] = (result > 255) ? 1 : 0;
                    chip8->V[x] = result;
                    break;
                }
                case 0x5:
                    // Subtract VY from VX, set VF to NOT borrow
                    chip8->V[0xF] = (chip8->V[x] >= chip8->V[y]) ? 1 : 0;
                    chip8->V[x] = chip8->V[x] - chip8->V[y];
                    break;
                case 0x6: {
                    // Shift VX right by 1, set VF to LSB before shift
                    uint8_t last_bit_right = chip8->V[x] & 0x1;
                    chip8->V[x] = chip8->V[x] >> 1;
                    chip8->V[0xF] = last_bit_right;
                    break;
                }
                case 0x7:
                    // Set VX to VY - VX, set VF to NOT borrow
                    chip8->V[0xF] = (chip8->V[y] >= chip8->V[x]) ? 1 : 0;
                    chip8->V[x] = chip8->V[y] - chip8->V[x];
                    break;
                case 0xE: {
                    // Shift VX left by 1, set VF to MSB before shift
                    uint8_t last_bit_left = chip8->V[x] >> 7;
                    chip8->V[x] = chip8->V[x] << 1;
                    chip8->V[0xF] = last_bit_left;
                    break;
                }
            }
            break;
        }

        case 0x9:
            // Skip next instruction if VX != VY
            if (nibbles[3] == 0) {
                if (chip8->V[nibbles[1]] != chip8->V[nibbles[2]]) {
                    chip8->pc += 2;
                }
            }
            break;

        case 0xA:
            // Set index register I to NNN
            chip8->I = last_nibbles;
            break;

        case 0xB:
            // Jump to address NNN + V0
            chip8->pc = last_nibbles + chip8->V[0];
            break;

        case 0xC:
            // Set VX to random byte AND NN
            chip8->V[nibbles[1]] = rand() & second_byte;
            break;

        case 0xD: {
            // Draw sprite at (VX, VY) with height N
            uint16_t sprite_height = nibbles[3];
            uint16_t start_x = chip8->V[nibbles[1]] % DISPLAY_WIDTH;
            uint16_t start_y = chip8->V[nibbles[2]] % DISPLAY_HEIGHT;
            bool collision_detected = false;

            for (int y = 0; y < sprite_height; y++) {
                uint8_t sprite_row = chip8->memory[chip8->I + y];
                
                for (int x = 0; x < 8; x++) {
                    uint8_t curr_x = start_x + x;
                    uint8_t curr_y = start_y + y;

                    if (curr_x >= DISPLAY_WIDTH || curr_y >= DISPLAY_HEIGHT) {
                        continue;
                    }

                    uint8_t curr_val = chip8->display[curr_y][curr_x];
                    chip8->display[curr_y][curr_x] ^= ((sprite_row >> (7 - x)) & 0x1);

                    if (curr_val == 1 && chip8->display[curr_y][curr_x] == 0) {
                        collision_detected = true;
                    }
                }
            }

            chip8->V[0xF] = collision_detected ? 1 : 0;
            break;
        }

        case 0xE: {
            // Skip based on key press
            uint8_t key = chip8->V[nibbles[1]];
            
            if (second_byte == 0x9E) {
                // Skip if key VX is pressed
                if (chip8->keypad[key] == 1) {
                    chip8->pc += 2;
                }
            } else if (second_byte == 0xA1) {
                // Skip if key VX is not pressed
                if (chip8->keypad[key] == 0) {
                    chip8->pc += 2;
                }
            }
            break;
        }

        case 0xF: {
            // Timer, memory, and font operations
            uint8_t x = nibbles[1];
            
            switch (second_byte) {
                case 0x07:
                    // Set VX to delay timer
                    chip8->V[x] = chip8->delay_timer;
                    break;
                    
                case 0x0A: {
                    // Wait for key press and store in VX
                    bool key_pressed = false;
                    for (int i = 0; i < 16; i++) {
                        if (chip8->keypad[i] == 1) {
                            chip8->V[x] = i;
                            key_pressed = true;
                            break;
                        }
                    }
                    if (!key_pressed) {
                        chip8->pc -= 2;
                    }
                    break;
                }
                    
                case 0x15:
                    // Set delay timer to VX
                    chip8->delay_timer = chip8->V[x];
                    break;
                    
                case 0x18:
                    // Set sound timer to VX
                    chip8->sound_timer = chip8->V[x];
                    break;
                    
                case 0x1E:
                    // Add VX to I
                    chip8->I += chip8->V[x];
                    chip8->V[0xF] = (chip8->I > 0xFFF) ? 1 : 0;
                    break;
                    
                case 0x29:
                    // Set I to location of sprite for digit VX
                    chip8->I = FONT_START_ADDRESS + FONT_SPRITE_SIZE * (chip8->V[x] & 0xF);
                    break;
                    
                case 0x33: {
                    // Store BCD representation of VX in memory at I, I+1, I+2
                    uint8_t val = chip8->V[x];
                    chip8->memory[chip8->I]     = val / 100;
                    chip8->memory[chip8->I + 1] = (val / 10) % 10;
                    chip8->memory[chip8->I + 2] = val % 10;
                    break;
                }
                    
                case 0x55:
                    // Store V0 to VX in memory starting at I
                    for (int i = 0; i <= x; i++) {
                        chip8->memory[chip8->I + i] = chip8->V[i];
                    }
                    break;
                    
                case 0x65:
                    // Load V0 to VX from memory starting at I
                    for (int i = 0; i <= x; i++) {
                        chip8->V[i] = chip8->memory[chip8->I + i];
                    }
                    break;
            }
            break;
        }
    }
}
