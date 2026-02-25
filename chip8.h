#ifndef CHIP8_H
#define CHIP8_H

// Header files
#include <stdint.h>



// Function Declarations

int loadGameInRam (int fd, struct chip8CPU *cpu);
void fetch ();
void decodeAndExecute ();



// Structs

// CPU Class
struct chip8CPU {
    uint8_t ram[4096];  // 4KB memory
    uint16_t pc;        // Program Counter
    uint8_t  V[16];     // General purpose registers V0- V16 ( V0-VF )
    uint16_t I;         // Index register (points to memory addresses)
    uint16_t stack[16]; // Return address stack
    uint8_t  sp;        // Stack pointer
};


#endif