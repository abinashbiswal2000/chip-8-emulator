#include <stdio.h>
#include <stdint.h>

typedef struct {
    uint8_t  memory[4096];  // The 4KB RAM
    uint8_t  V[16];         // The 16 General Purpose Registers (V0-VF)
    uint16_t I;             // The Index Register (for memory addresses)
    uint16_t pc;            // The Program Counter
    uint16_t stack[16];     // The Stack (for subroutines)
    uint8_t  sp;            // The Stack Pointer
    
} Chip8;

int main () {
    return 0;
}