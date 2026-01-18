#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

// CHIP-8 system constants
#define MEMORY_SIZE 4096
#define REGISTER_COUNT 16
#define STACK_SIZE 16
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define KEYPAD_SIZE 16
#define PROGRAM_START 0x200
#define FONT_START_ADDRESS 0x050
#define FONT_SPRITE_SIZE 5

typedef struct {
    uint8_t memory[MEMORY_SIZE];
    uint8_t V[REGISTER_COUNT];
    uint16_t I;
    uint16_t pc;
    uint8_t sp;
    uint16_t stack[STACK_SIZE];
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t display[DISPLAY_HEIGHT][DISPLAY_WIDTH];
    uint8_t keypad[KEYPAD_SIZE];
} Chip8;

// Public API
void chip8_init(Chip8 *chip8);
bool chip8_load_rom(Chip8 *chip8, const char *filepath);
void chip8_execute_cycle(Chip8 *chip8);
void chip8_update_timers(Chip8 *chip8);

#endif
