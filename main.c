#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

// SDL values
SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Renderer* gRenderer = NULL;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 320;

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

// Font
uint8_t font[80] = {
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

// SDL functions
void render_display();  
bool sdl_init();        
void sdl_close();

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

    // Keypad
    memset(keypad, 0, sizeof(keypad));

    // Font
    int i ;
    for (i = 0; i < 80; i++) {
        chip8ram[i + 0x050] = font[i];
    };

    return 0;
}

int load_rom(char *filepath) {
    FILE *fptr;

    fptr = fopen(filepath, "rb");

    if (fptr == NULL) {
        printf("Error opening file. Exiting program.\n");
        return 0;
    };

    // Get file size
    fseek(fptr, 0, SEEK_END);

    long file_size = ftell(fptr);

    fseek(fptr, 0, SEEK_SET);

    size_t bytes_read = fread(chip8ram + 0x200, 1, file_size, fptr);

    fclose(fptr);

    return 1;
}

void loop() {
    bool is_running = true;
    int count = 0;
    SDL_Event e;

    uint32_t last_timer_update = SDL_GetTicks();

    while (is_running) {
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) {
                is_running = false;
            } 
            else if(e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.sym) {
                    case SDLK_1: keypad[0x1] = 1; break;
                    case SDLK_2: keypad[0x2] = 1; break;
                    case SDLK_3: keypad[0x3] = 1; break;
                    case SDLK_4: keypad[0xC] = 1; break;
                    case SDLK_q: keypad[0x4] = 1; break;
                    case SDLK_w: keypad[0x5] = 1; break;
                    case SDLK_e: keypad[0x6] = 1; break;
                    case SDLK_r: keypad[0xD] = 1; break;
                    case SDLK_a: keypad[0x7] = 1; break;
                    case SDLK_s: keypad[0x8] = 1; break;
                    case SDLK_d: keypad[0x9] = 1; break;
                    case SDLK_f: keypad[0xE] = 1; break;
                    case SDLK_z: keypad[0xA] = 1; break;
                    case SDLK_x: keypad[0x0] = 1; break;
                    case SDLK_c: keypad[0xB] = 1; break;
                    case SDLK_v: keypad[0xF] = 1; break;
                }
            }
            else if(e.type == SDL_KEYUP) {
                switch(e.key.keysym.sym) {
                    case SDLK_1: keypad[0x1] = 0; break;
                    case SDLK_2: keypad[0x2] = 0; break;
                    case SDLK_3: keypad[0x3] = 0; break;
                    case SDLK_4: keypad[0xC] = 0; break;
                    case SDLK_q: keypad[0x4] = 0; break;
                    case SDLK_w: keypad[0x5] = 0; break;
                    case SDLK_e: keypad[0x6] = 0; break;
                    case SDLK_r: keypad[0xD] = 0; break;
                    case SDLK_a: keypad[0x7] = 0; break;
                    case SDLK_s: keypad[0x8] = 0; break;
                    case SDLK_d: keypad[0x9] = 0; break;
                    case SDLK_f: keypad[0xE] = 0; break;
                    case SDLK_z: keypad[0xA] = 0; break;
                    case SDLK_x: keypad[0x0] = 0; break;
                    case SDLK_c: keypad[0xB] = 0; break;
                    case SDLK_v: keypad[0xF] = 0; break;
                }
            }
        }

        // Fetch next instruction
        uint16_t opcode = (chip8ram[pc] << 8) | chip8ram[pc + 1];

        // Increment pc to be ready to fetch next opcode
        pc += 2;

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
                        memset(display, 0, sizeof(display));
                        break;
                    case 0xEE:
                        // Return from subroutine
                        sp--;
                        pc = stack[sp];
                        break;
                }
                break;

            case 0x1:
                // Jump to address NNN
                pc = last_nibbles;
                break;

            case 0x2:
                // Call subroutine at NNN
                stack[sp] = pc;
                sp++;
                pc = last_nibbles;
                break;

            case 0x3:
                // Skip next instruction if VX == NN
                if (gp_registers[nibbles[1]] == second_byte) {
                    pc += 2;
                }
                break;

            case 0x4:
                // Skip next instruction if VX != NN
                if (gp_registers[nibbles[1]] != second_byte) {
                    pc += 2;
                }
                break;

            case 0x5:
                // Skip next instruction if VX == VY
                if (nibbles[3] == 0) {
                    if (gp_registers[nibbles[1]] == gp_registers[nibbles[2]]) {
                        pc += 2;
                    }
                }
                break;

            case 0x6:
                // Set VX to NN
                gp_registers[nibbles[1]] = second_byte;
                break;

            case 0x7:
                // Add NN to VX
                gp_registers[nibbles[1]] += second_byte;
                break;

            case 0x8: {
                // Arithmetic and logic operations
                uint8_t x = nibbles[1];
                uint8_t y = nibbles[2];

                switch (nibbles[3]) {
                    case 0x0:
                        // Set VX to VY
                        gp_registers[x] = gp_registers[y];
                        break;
                    case 0x1:
                        // Set VX to VX OR VY
                        gp_registers[x] = gp_registers[x] | gp_registers[y];
                        break;
                    case 0x2:
                        // Set VX to VX AND VY
                        gp_registers[x] = gp_registers[x] & gp_registers[y];
                        break;
                    case 0x3:
                        // Set VX to VX XOR VY
                        gp_registers[x] = gp_registers[x] ^ gp_registers[y];
                        break;
                    case 0x4: {
                        // Add VY to VX, set VF to carry
                        uint16_t result = gp_registers[x] + gp_registers[y];
                        gp_registers[0xF] = (result > 255) ? 1 : 0;
                        gp_registers[x] = result;
                        break;
                    }
                    case 0x5:
                        // Subtract VY from VX, set VF to NOT borrow
                        gp_registers[0xF] = (gp_registers[x] >= gp_registers[y]) ? 1 : 0;
                        gp_registers[x] = gp_registers[x] - gp_registers[y];
                        break;
                    case 0x6: {
                        // Shift VX right by 1, set VF to LSB before shift
                        uint8_t last_bit_right = gp_registers[x] & 0x1;
                        gp_registers[x] = gp_registers[x] >> 1;
                        gp_registers[0xF] = last_bit_right;
                        break;
                    }
                    case 0x7:
                        // Set VX to VY - VX, set VF to NOT borrow
                        gp_registers[0xF] = (gp_registers[y] >= gp_registers[x]) ? 1 : 0;
                        gp_registers[x] = gp_registers[y] - gp_registers[x];
                        break;
                    case 0xE: {
                        // Shift VX left by 1, set VF to MSB before shift
                        uint8_t last_bit_left = gp_registers[x] >> 7;
                        gp_registers[x] = gp_registers[x] << 1;
                        gp_registers[0xF] = last_bit_left;
                        break;
                    }
                }
                break;
            }

            case 0x9:
                // Skip next instruction if VX != VY
                if (nibbles[3] == 0) {
                    if (gp_registers[nibbles[1]] != gp_registers[nibbles[2]]) {
                        pc += 2;
                    }
                }
                break;

            case 0xA:
                // Set index register I to NNN
                I = last_nibbles;
                break;

            case 0xB:
                // Jump to address NNN + V0
                pc = last_nibbles + gp_registers[0];
                break;

            case 0xC:
                // Set VX to random byte AND NN
                gp_registers[nibbles[1]] = rand() & second_byte;
                break;

            case 0xD: {
                // Draw sprite at (VX, VY) with height N
                uint16_t sprite_height = nibbles[3];
                uint16_t start_x = gp_registers[nibbles[1]] % 64;
                uint16_t start_y = gp_registers[nibbles[2]] % 32;
                bool collision_detected = false;

                for (int y = 0; y < sprite_height; y++) {
                    uint8_t sprite_row = chip8ram[I + y];
                    
                    for (int x = 0; x < 8; x++) {
                        uint8_t curr_x = start_x + x;
                        uint8_t curr_y = start_y + y;

                        if (curr_x >= 64 || curr_y >= 32) {
                            continue;
                        }

                        uint8_t curr_val = display[curr_y][curr_x];
                        display[curr_y][curr_x] ^= ((sprite_row >> (7 - x)) & 0x1);

                        if (curr_val == 1 && display[curr_y][curr_x] == 0) {
                            collision_detected = true;
                        }
                    }
                }

                gp_registers[0xF] = collision_detected ? 1 : 0;
                break;
            }

            case 0xE: {
                // Skip based on key press
                uint8_t key = gp_registers[nibbles[1]];
                
                if (second_byte == 0x9E) {
                    // Skip if key VX is pressed
                    if (keypad[key] == 1) {
                        pc += 2;
                    }
                } else if (second_byte == 0xA1) {
                    // Skip if key VX is not pressed
                    if (keypad[key] == 0) {
                        pc += 2;
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
                        gp_registers[x] = delay_timer;
                        break;
                        
                    case 0x0A: {
                        // Wait for key press and store in VX
                        bool key_pressed = false;
                        for (int i = 0; i < 16; i++) {
                            if (keypad[i] == 1) {
                                gp_registers[x] = i;
                                key_pressed = true;
                                break;
                            }
                        }
                        if (!key_pressed) {
                            pc -= 2;
                        }
                        break;
                    }
                        
                    case 0x15:
                        // Set delay timer to VX
                        delay_timer = gp_registers[x];
                        break;
                        
                    case 0x18:
                        // Set sound timer to VX
                        sound_timer = gp_registers[x];
                        break;
                        
                    case 0x1E:
                        // Add VX to I
                        I += gp_registers[x];
                        gp_registers[0xF] = (I > 0xFFF) ? 1 : 0;
                        break;
                        
                    case 0x29:
                        // Set I to location of sprite for digit VX
                        I = 0x050 + 5 * (gp_registers[x] & 0xF);
                        break;
                        
                    case 0x33: {
                        // Store BCD representation of VX in memory at I, I+1, I+2
                        uint8_t val = gp_registers[x];
                        chip8ram[I]     = val / 100;
                        chip8ram[I + 1] = (val / 10) % 10;
                        chip8ram[I + 2] = val % 10;
                        break;
                    }
                        
                    case 0x55:
                        // Store V0 to VX in memory starting at I
                        for (int i = 0; i <= x; i++) {
                            chip8ram[I + i] = gp_registers[i];
                        }
                        break;
                        
                    case 0x65:
                        // Load V0 to VX from memory starting at I
                        for (int i = 0; i <= x; i++) {
                            gp_registers[i] = chip8ram[I + i];
                        }
                        break;
                }
                break;
            }
        }

        render_display();
        SDL_Delay(8);

        uint32_t current_time = SDL_GetTicks();
        if(current_time - last_timer_update >= 16) {
            if(delay_timer > 0) delay_timer--;
            if(sound_timer > 0) sound_timer--;
            last_timer_update = current_time;
        }
    }
}

bool sdl_init() {
    bool success = true;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        success = false;
    }
    else {
        gWindow = SDL_CreateWindow("CHIP-8 Emulator", 
                                   SDL_WINDOWPOS_UNDEFINED, 
                                   SDL_WINDOWPOS_UNDEFINED, 
                                   SCREEN_WIDTH, 
                                   SCREEN_HEIGHT, 
                                   SDL_WINDOW_SHOWN);
        if (gWindow == NULL) {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            success = false;
        }
        else {
            // Create renderer
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
            if (gRenderer == NULL) {
                printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            }
        }
    }

    return success;
}

void render_display() {
    // Clear screen (black background)
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);

    // Set draw color to white for pixels
    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);

    // Draw each pixel
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (display[y][x] == 1) {
                // Each CHIP-8 pixel is 10x10 screen pixels
                SDL_Rect pixel = {
                    x * 10,  // x position
                    y * 10,  // y position
                    10,      // width
                    10       // height
                };
                SDL_RenderFillRect(gRenderer, &pixel);
            }
        }
    }

    // Update screen
    SDL_RenderPresent(gRenderer);
}

void sdl_close() {
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gRenderer = NULL;
    gWindow = NULL;
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <rom_file>\n", argv[0]);
        printf("Example: %s invaders.ch8\n", argv[0]);
        return 1;
    }

    if (!sdl_init()) {
        printf("Failed to initialize SDL!\n");
        return 1;
    }

    srand(time(NULL));
    initialize_values();
    
    if (!load_rom(argv[1])) {
        printf("Failed to load ROM: %s\n", argv[1]);
        sdl_close();
        return 1;
    }

    loop();
    
    sdl_close();
    return 0;
}IOPOL_ATIME_UPDATES_DEFAULT
