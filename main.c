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

    // Initializing instruction and decodedInstruction variables
    uint16_t instruction;
    struct decodedInstructionStruct decodedInstruction = {0};
    struct decodedInstructionStruct *decodedInstructionPtr = &decodedInstruction;
    
    // ( Fetch - Decode - Execute ) cycle
    while (1) {
        instruction = fetch(cpuPtr);
        decodeInstruction(decodedInstructionPtr, instruction);
    }
    
    return 0;
 }