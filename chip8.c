#include "chip8.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>


void initializeCpu(struct chip8CPU *cpuPtr) {
    /*
    1 - Set every byte in the ram to zero.
    Why?
    To get rid of all garbage values
    Why ?
    The game developers wrote their ROMs assuming they were running on a CHIP-8 machine that boots into a known, clean state. They never bothered writing code to explicitly zero out memory before using it, because the hardware already guaranteed that. So when you emulate that machine, you have to uphold that same guarantee — otherwise you're giving the ROM a different environment than it was written for, and we can expect undefined behaviour.
    
    2 - Set Pc to 0x200 (512)
    Why?
    Because all the instructions are stored from the location

    3 - Load font set in the beginning of the ram.
    */

    
    struct chip8CPU temp = {0};
    *cpuPtr = temp;

    cpuPtr->pc = 0x200;

    for (int i = 0; i < 80; i++) {
        cpuPtr->ram[i] = fontset[i];
    }

};


int loadGameInRam (char *fileName, struct chip8CPU *cpuPtr) {

    int fileDescriptor = open(fileName, O_RDONLY);

    if (fileDescriptor < 0) {
        printf("File Not Found\n");
        return 0;
    }
    
    uint8_t buffer[1000];
    ssize_t bytesRead;
    
    int totalBytesRead = 0;
    int ramIndex = 512;

    while (1) {

        bytesRead = read(fileDescriptor, buffer, 1000);

        if (bytesRead == 0) {
            printf("Game file read successfully\ntotalBytesRead = %d\n", totalBytesRead);
            return 1;
            // file reading completed
        } else if (bytesRead < 0) {
            printf("Input device disconnected\n");
            return 0;
            // Input device disconnected
        }
        
        totalBytesRead += bytesRead;

        if (totalBytesRead > 3584) {
            printf("Game file too large\n");
            return 0;
            // Game file too large
        }

        for (int i = 0; i < bytesRead; i++) {
            cpuPtr->ram[ramIndex] = buffer[i];
            ramIndex += 1;
        }

    }
};


uint16_t fetch (struct chip8CPU *cpuPtr) {
    
    // Chip 8 is big endian
    // Size of an instruction in chip 8 is 2 bytes = 16 bits
    
    uint8_t leftPart = cpuPtr->ram[cpuPtr->pc];
    uint8_t rightPart = cpuPtr->ram[cpuPtr->pc + 1];
    uint16_t instruction = (leftPart << 8) | rightPart;

    cpuPtr->pc += 2;
    
    return instruction;
};


void decodeAndExecute () {};


void decodeInstruction (struct decodedInstructionStruct *decodedInstructionStructPtr, uint16_t instruction) {
    decodedInstructionStructPtr->opcode = (instruction & 0xF000) >> 12;
    decodedInstructionStructPtr->x      = (instruction & 0x0F00) >> 8;
    decodedInstructionStructPtr->y      = (instruction & 0x00F0) >> 4;
    decodedInstructionStructPtr->n      = (instruction & 0x000F);
    decodedInstructionStructPtr->kk     = (instruction & 0x00FF);
    decodedInstructionStructPtr->nnn    = (instruction & 0x0FFF);
}


// ------------------------------------------------------------------------


// Font Set
uint8_t fontset[80] = {
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