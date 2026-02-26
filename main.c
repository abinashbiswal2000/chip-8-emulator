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

        printf("pc before fetch = %02X %02X\n", cpu.ram[cpu.pc], cpu.ram[cpu.pc+1]);
        instruction = fetch(cpuPtr);
        printf("pc after fetch  = %02X %02X\n", cpu.ram[cpu.pc], cpu.ram[cpu.pc+1]);
        decodeInstruction(decodedInstructionPtr, instruction);

        printf("Opcode = %01X \n", decodedInstruction.opcode);
        printf("     x = %01X \n", decodedInstruction.x);
        printf("     y = %01X \n", decodedInstruction.y);
        printf("     n = %01X \n", decodedInstruction.n);
        printf("    kk = %02X \n", decodedInstruction.kk);
        printf("   nnn = %03X \n", decodedInstruction.nnn);

        break;

        if (cpuPtr->pc >= 4096) break; 
    }
    
    return 0;
 }