

#include <stdio.h>
#include <SDL2/SDL.h>
#include "chip8.h"

#define SCALE 15
#define WIDTH 64
#define HEIGHT 32

void renderDisplay(SDL_Renderer *renderer, struct chip8CPU *cpuPtr) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            SDL_Rect rect = {
                x * SCALE,
                y * SCALE,
                SCALE,
                SCALE
            };
            if (cpuPtr->display[y * WIDTH + x] == 1) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // white
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);       // black
            }
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    SDL_RenderPresent(renderer);
}


int main (int argc, char *argv[]) {

    if (argc != 2) {
        printf("Incorrect Number of Arguments\n");
        return 1;
    }

    // Initialize CPU
    struct chip8CPU cpu;
    struct chip8CPU *cpuPtr = &cpu;
    initializeCpu(cpuPtr);

    // Load Game
    if (loadGameInRam(argv[1], cpuPtr) == 0) {
        return 1;
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH * SCALE, HEIGHT * SCALE,
        0
    );

    if (window == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Instruction variables
    uint16_t instruction;
    struct decodedInstructionStruct decodedInstruction = {0};
    struct decodedInstructionStruct *decodedInstructionPtr = &decodedInstruction;

    // Game loop
    SDL_Event event;
    int running = 1;

    while (running) {

        // 1 - Poll events
        // while (SDL_PollEvent(&event)) {
        //     if (event.type == SDL_QUIT) {
        //         running = 0;
        //     }
        // }
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            
            // Keydown: Set the specific key to 1
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_w) cpuPtr->keys[1] = 1; // Example mapping
                if (event.key.keysym.sym == SDLK_s) cpuPtr->keys[4] = 1;
            }
            
            // Keyup: Reset the specific key to 0
            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_w) cpuPtr->keys[1] = 0;
                if (event.key.keysym.sym == SDLK_s) cpuPtr->keys[4] = 0;
            }
        }

        // 2 - Run 10 CPU cycles
        for (int i = 0; i < 10; i++) {
            instruction = fetch(cpuPtr);
            decodeInstruction(decodedInstructionPtr, instruction);
            if (execute(decodedInstructionPtr, cpuPtr) == 0) {
                printf("Execution Stopped\n");
                running = 0;
                break;
            }
        }

        // 3 - Decrement timers
        if (cpuPtr->DT > 0) cpuPtr->DT--;
        if (cpuPtr->ST > 0) cpuPtr->ST--;

        // 4 - Render display
        renderDisplay(renderer, cpuPtr);

        // 5 - Cap at 60fps
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}