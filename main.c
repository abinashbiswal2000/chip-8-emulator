#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include "chip8.h"


int main (int argc, char *argv[]) {

    if (argc != 2) {
        printf("Incorrect Number of Arguments\n");
        return 1;
    }
    
    char *nameOfFile = argv[1];
    int fd = open(nameOfFile, O_RDONLY);

    if (fd < 0) {
        printf("File Not Found\n");
        return 1;
    }

    struct chip8CPU cpu;
    

    /*    
    // A simplified loop for testing
    int running = 1;
    while (running) {
        // uint16_t opcode = fetch(&cpu);
        // decode_and_execute(&cpu, opcode);

        // Instead of a screen, we use a "Debug Print"
        // printf("PC: %04X | Opcode: %04X | V0: %02X | V1: %02X\n", cpu.pc - 2, opcode, cpu.V[0], cpu.V[1]);
                
        // Add a way to stop the loop, or it will run forever!
        if (cpu.pc >= 4096) running = 0;
    }    
    */   
    
    return 0;
}