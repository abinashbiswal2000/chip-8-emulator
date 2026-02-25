#include <stdio.h>
#include "chip8.h"

int main (int argc, char *argv[]) {

    if (argc != 2) {
        printf("Incorrect Number of Arguments\n");
        return 1;
    }

    // Inititlize CPU
    struct chip8CPU cpu;
    struct chip8CPU *cpuPtr = &cpu;
    initializeCpu(cpuPtr);

    // Load Game in Virtual cpu's ram
    if (loadGameInRam(argv[1], cpuPtr) == 0) {
        return 1;
    }
    
    return 0;
 }