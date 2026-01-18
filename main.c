#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "chip8.h"
#include <SDL2/SDL.h>

// SDL values
SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Renderer* gRenderer = NULL;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 320;

// SDL functions
void render_display(Chip8 *chip8);  
bool sdl_init();        
void sdl_close();

void loop(Chip8 *chip8) {
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
                    case SDLK_1: chip8->keypad[0x1] = 1; break;
                    case SDLK_2: chip8->keypad[0x2] = 1; break;
                    case SDLK_3: chip8->keypad[0x3] = 1; break;
                    case SDLK_4: chip8->keypad[0xC] = 1; break;
                    case SDLK_q: chip8->keypad[0x4] = 1; break;
                    case SDLK_w: chip8->keypad[0x5] = 1; break;
                    case SDLK_e: chip8->keypad[0x6] = 1; break;
                    case SDLK_r: chip8->keypad[0xD] = 1; break;
                    case SDLK_a: chip8->keypad[0x7] = 1; break;
                    case SDLK_s: chip8->keypad[0x8] = 1; break;
                    case SDLK_d: chip8->keypad[0x9] = 1; break;
                    case SDLK_f: chip8->keypad[0xE] = 1; break;
                    case SDLK_z: chip8->keypad[0xA] = 1; break;
                    case SDLK_x: chip8->keypad[0x0] = 1; break;
                    case SDLK_c: chip8->keypad[0xB] = 1; break;
                    case SDLK_v: chip8->keypad[0xF] = 1; break;
                }
            }
            else if(e.type == SDL_KEYUP) {
                switch(e.key.keysym.sym) {
                    case SDLK_1: chip8->keypad[0x1] = 0; break;
                    case SDLK_2: chip8->keypad[0x2] = 0; break;
                    case SDLK_3: chip8->keypad[0x3] = 0; break;
                    case SDLK_4: chip8->keypad[0xC] = 0; break;
                    case SDLK_q: chip8->keypad[0x4] = 0; break;
                    case SDLK_w: chip8->keypad[0x5] = 0; break;
                    case SDLK_e: chip8->keypad[0x6] = 0; break;
                    case SDLK_r: chip8->keypad[0xD] = 0; break;
                    case SDLK_a: chip8->keypad[0x7] = 0; break;
                    case SDLK_s: chip8->keypad[0x8] = 0; break;
                    case SDLK_d: chip8->keypad[0x9] = 0; break;
                    case SDLK_f: chip8->keypad[0xE] = 0; break;
                    case SDLK_z: chip8->keypad[0xA] = 0; break;
                    case SDLK_x: chip8->keypad[0x0] = 0; break;
                    case SDLK_c: chip8->keypad[0xB] = 0; break;
                    case SDLK_v: chip8->keypad[0xF] = 0; break;
                }
            }
        }

        chip8_execute_cycle(chip8);

        render_display(chip8);
        SDL_Delay(8);

        uint32_t current_time = SDL_GetTicks();
        if(current_time - last_timer_update >= 16) {
            chip8_update_timers(chip8);
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

void render_display(Chip8 *chip8) {
    // Clear screen (black background)
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);

    // Set draw color to white for pixels
    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);

    // Draw each pixel
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (chip8->display[y][x] == 1) {
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

    Chip8 chip8;
    chip8_init(&chip8);
    
    if (!chip8_load_rom(&chip8, argv[1])) {
        printf("Failed to load ROM: %s\n", argv[1]);
        sdl_close();
        return 1;
    }

    loop(&chip8);
    
    sdl_close();
    return 0;
}
