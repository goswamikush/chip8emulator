#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

// SDL constants
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
                    case SDLK_5: keypad[0x5] = 1; break;  // Add in KEYDOWN
                    case SDLK_6: keypad[0x6] = 1; break;
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
        nibbles[1] = opcode >> 8 & 0xF;
        nibbles[2] = opcode >> 4 & 0xF;
        nibbles[3] = opcode & 0xF;

        // printf("PC: %04X | Opcode: %04X | First nibble: %X\n", pc - 2, opcode, nibbles[0]);

        uint16_t second_byte = (nibbles[2] << 4) | nibbles[3];
        uint16_t last_nibbles = (nibbles[1] << 8) | (nibbles[2] << 4) | nibbles[3];

        // Clear screen and subroutine
        if (nibbles[0] == 0 && nibbles[1] == 0) {
            if (second_byte == 0xE0) {
                memset(display, 0, sizeof(display)); 
            } else if (second_byte == 0xEE) {
                sp--;
                uint16_t return_addr = stack[sp];
                pc = return_addr;
            }
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

        // Timers, Add to Index, Font
        if (nibbles[0] == 0xF) {
            uint8_t *reg = &gp_registers[nibbles[1]];

            if (second_byte == 0x07) {
                *reg = delay_timer;
            } else if (second_byte == 0x15) {
                delay_timer = *reg;
            } else if (second_byte == 0x18) {
                sound_timer = *reg;
            } else if (second_byte == 0x1E) {
                // Add to index
                I += *reg;

                if (I > 0xFFF) {
                    gp_registers[0xF] = 1;
                } else {
                    gp_registers[0xF] = 0;
                }
            } else if (second_byte == 0x33) {
                uint8_t digits[3];
                
                uint8_t val = *reg;
                int i;
                for (i = 0; i < 3; i++) {
                    digits[2 - i] = val % 10;
                    val = val / 10;
                };
                // Place digits in spots
                int j;
                for (j = 0; j < 3; j++) {
                    chip8ram[I + j] = digits[j];
                };
            } else if (second_byte == 0x33) {
                uint8_t digits[3];
                
                uint8_t val = *reg;
                int i;
                for (i = 0; i < 3; i++) {
                    uint8_t curr_digit = val % 10;
                    digits[3 - i] = curr_digit;
                    val = (val - curr_digit) / 10;
                };
                // Place digits in spots
                int j;
                for (j = 0; j < 3; j++) {
                    chip8ram[I + j] = digits[j];
                };
            } else if (second_byte == 55) {
                int i;
                for (i = 0; i <= nibbles[1]; i++) {
                    uint8_t curr_gp = gp_registers[i];

                    chip8ram[I + i] = curr_gp;
                };
            } else if (second_byte == 0x65) {
                int i;
                for (i = 0; i <= nibbles[1]; i++) {
                    uint8_t curr_val = chip8ram[I + i];

                    gp_registers[i] = curr_val;
                };
            } else if (second_byte == 0x29) {
                I = 0x050 + 5 * (*reg & 0xF);
            } else if (second_byte == 0x0A) {
                // Wait for key press
                bool key_pressed = false;
                for (int i = 0; i < 16; i++) {
                    if (keypad[i] == 1) {
                        *reg = i;
                        key_pressed = true;
                        break;
                    };
                };
                // If no key pressed, repeat this instruction
                if (!key_pressed) {
                    pc -= 2;
                };
            };
        };

        // Call subroutine
        if (nibbles[0] == 2) {
            stack[sp] = pc;
            sp++;
            pc = last_nibbles;
        };

        // Skip conditionally
        if (nibbles[0] == 3) {
            if (gp_registers[nibbles[1]] == second_byte) {
                pc += 2;
            };
        };

        if (nibbles[0] == 4) {
            if (gp_registers[nibbles[1]] != second_byte) {
                pc += 2;
            };
        };

        if (nibbles[0] == 5 && nibbles[3] == 0) {
            if (gp_registers[nibbles[1]] == gp_registers[nibbles[2]]) {
                pc += 2;
            };
        };

        if (nibbles[0] == 9 && nibbles[3] == 0) {
            if (gp_registers[nibbles[1]] != gp_registers[nibbles[2]]) {
                pc += 2;
            };
        };
        
        // Arithmetic
        if (nibbles[0] == 8) {
            uint8_t last_nibble = nibbles[3];

            uint8_t x = nibbles[1];
            uint8_t y = nibbles[2];

            // Set
            switch (last_nibble) {
                case 0:
                    gp_registers[x] = gp_registers[y];
                    break;
                case 1:
                    gp_registers[x] = gp_registers[x] | gp_registers[y];
                    break;
                case 2:
                    gp_registers[x] = gp_registers[x] & gp_registers[y];
                    break;
                case 3:
                    gp_registers[x] = gp_registers[x] ^ gp_registers[y];
                    break;
                case 4: {
                    uint16_t result = gp_registers[x] + gp_registers[y];
                    if (result > 255) {
                        gp_registers[0xF] = 1;
                    } else {
                        gp_registers[0xF] = 0;
                    }
                    gp_registers[x] = result;
                    break;
                }
                case 5:
                    if (gp_registers[x] >= gp_registers[y]) {
                        gp_registers[0xF] = 1;
                    } else {
                        gp_registers[0xF] = 0;
                    }

                    gp_registers[x] = gp_registers[x] - gp_registers[y];
                    break;
                case 7:
                    if (gp_registers[y] >= gp_registers[x]) {
                        gp_registers[0xF] = 1;
                    } else {
                        gp_registers[0xF] = 0;
                    }

                    gp_registers[x] = gp_registers[y] - gp_registers[x];
                    break;                    
                case 6: {
                    uint8_t last_bit_right = gp_registers[x] & 0x1;

                    gp_registers[x] = gp_registers[x] >> 1;

                    gp_registers[0xF] = last_bit_right;
                    break;
                }
                case 0xE: {
                    uint8_t last_bit_left = gp_registers[x] >> 7;

                    gp_registers[x] = gp_registers[x] << 1;
                    
                    gp_registers[0xF] = last_bit_left;
                    break;                
                }
            }
        };
        
        // Jump with offset
        if (nibbles[0] == 0xB) {
            pc = last_nibbles + gp_registers[0];
        };

        // Random
        if (nibbles[0] == 0xC) {
            uint8_t random_number = rand();
            gp_registers[nibbles[1]] = random_number & second_byte;
        };

        // Skip if key
        if (nibbles[0] == 0xE) {
            uint8_t x = gp_registers[nibbles[1]];
            if (second_byte == 0x9E) {
                if (keypad[x] == 1) {
                    pc += 2;
                };
            } else if (second_byte == 0xA1) {
                if (keypad[x] == 0) {
                    pc += 2;
                };
            };
        };

        render_display();
        SDL_Delay(8);

        uint32_t current_time = SDL_GetTicks();
        if(current_time - last_timer_update >= 16) {  // ~60Hz (1000ms/60 ≈ 16ms)
            if(delay_timer > 0) delay_timer--;
            if(sound_timer > 0) sound_timer--;
            last_timer_update = current_time;
        }
    };
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

    // printf("First 10 bytes of ROM:\n");
    // for(int i = 0; i < 10; i++) {
    //     printf("%02X ", chip8ram[0x200 + i]);
    // }
    // printf("\n");
    
    loop();
    
    sdl_close();
    return 0;
}

// gcc main.c -o chip8 -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2

