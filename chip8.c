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
};


int execute(struct decodedInstructionStruct *dPtr, struct chip8CPU *cPtr) {
    switch (dPtr->opcode) {
        case (0x0): {
            switch (dPtr->kk) {
                case (0xE0): {
                    // CLS - Clear the display
                    break;
                }
                case (0xEE): {
                    // RET - Return from a subroutine
                    cPtr->pc = cPtr->stack[cPtr->sp];
                    cPtr->sp -= 1;
                    break;
                }
                default: {
                    break;
                }
            }
            break;
        }
        
        case (0x1): {
            // 1nnn - JP addr -> Jump to location nnn.
            // The interpreter sets the program counter to nnn.
            cPtr->pc = dPtr->nnn;
            break;
        }

        case (0x2): {
            // 2nnn - CALL addr -> Call subroutine at nnn. 
            // 1. The interpreter increments the stack pointer, 
            // 2. Then puts the current PC on the top of the stack. 
            // 3. The PC is then set to nnn.

            if (cPtr->sp == 15) {
                printf("Stack Overflow\n");
                return 0;
            }

            cPtr->sp += 1;
            cPtr->stack[cPtr->sp] = cPtr->pc;
            cPtr->pc = dPtr->nnn;
            break;
        }

        case (0x3): {
            // 3xkk - SE Vx, byte -> Skip next instruction if Vx = kk.
            // The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
            if (cPtr->V[dPtr->x] == dPtr->kk) {
                cPtr->pc += 2;
            }
            break;
        }

        case (0x4): {
            // 4xkk - SNE Vx, byte -> Skip next instruction if Vx != kk.
            // The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.
            if (cPtr->V[dPtr->x] != dPtr->kk) {
                cPtr->pc += 2;
            }
            break;
        }

        case (0x5): {
            // 5xy0 - SE Vx, Vy -> Skip next instruction if Vx = Vy.
            // The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
            if (cPtr->V[dPtr->x] == cPtr->V[dPtr->y]) {
                cPtr->pc += 2;
            }
            break;
        }

        case (0x6): {
            // 6xkk - LD Vx, byte -> Set Vx = kk.
            // The interpreter puts the value kk into register Vx.
            cPtr->V[dPtr->x] = dPtr->kk;
            break;
        }

        case (0x7): {
            // 7xkk - ADD Vx, byte -> Set Vx = Vx + kk.
            // Adds the value kk to the value of register Vx, then stores the result in Vx.
            cPtr->V[dPtr->x] += dPtr->kk;
            break;
        }

        case (0x8): {
            switch (dPtr->n) {
                case (0x0): {
                    // 8xy0 - LD Vx, Vy -> Set Vx = Vy.
                    // Stores the value of register Vy in register Vx.
                    cPtr->V[dPtr->x] = cPtr->V[dPtr->y];
                    break;
                }
                case (0x1): {
                    // Set Vx = Vx OR Vy.
                    // Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx. 
                    cPtr->V[dPtr->x] = cPtr->V[dPtr->x] | cPtr->V[dPtr->y];
                    break;
                }
                case (0x2): {
                    // 8xy2 - AND Vx, VySet Vx = Vx AND Vy.
                    // Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
                    cPtr->V[dPtr->x] = cPtr->V[dPtr->x] & cPtr->V[dPtr->y];
                    break;
                }
                case (0x3): {
                    // 8xy3 - XOR Vx, Vy
                    // Set Vx = Vx XOR Vy.
                    cPtr->V[dPtr->x] = cPtr->V[dPtr->x] ^ cPtr->V[dPtr->y];
                    break;
                }
                case (0x4): {
                    // 8xy4 - ADD Vx, Vy -> Set Vx = Vx + Vy, set VF = carry.
                    // The values of Vx and Vy are added together. If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0. Only the lowest 8 bits of the result are kept, and stored in Vx.
                    uint16_t sum = cPtr->V[dPtr->x] + cPtr->V[dPtr->y];
                    uint8_t carry = sum > 255 ? 1 : 0;
                    cPtr->V[dPtr->x] = sum & 0xFF;
                    cPtr->V[0xF] = carry;
                    break;
                }
                case (0x5): {
                    // 8xy5 - SUB Vx, Vy -> Set Vx = Vx - Vy, set VF = NOT borrow.
                    // If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
                    uint8_t carry = cPtr->V[dPtr->x] >= cPtr->V[dPtr->y] ? 1 : 0;
                    cPtr->V[dPtr->x] -= cPtr->V[dPtr->y];
                    cPtr->V[0xF] = carry;  

                    break;
                }
                case (0x6): {
                    // 8xy6 - SHR Vx {, Vy} -> Set Vx = Vx SHR 1.
                    // If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
                    uint8_t lsb = cPtr->V[dPtr->x] & 0x1;
                    cPtr->V[dPtr->x] = cPtr->V[dPtr->x] / 2;
                    cPtr->V[0xF] = lsb;                    
                    break;
                }
                case (0x7): {
                    // 8xy7 - SUBN Vx, Vy -> Set Vx = Vy - Vx, set VF = NOT borrow.
                    // If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
                    uint8_t carry = cPtr->V[dPtr->y] > cPtr->V[dPtr->x] ? 1 : 0;
                    cPtr->V[dPtr->x] = cPtr->V[dPtr->y] - cPtr->V[dPtr->x];
                    cPtr->V[0xF] = carry;
                    break;
                }
                case (0xE): {
                    // Set Vx = Vx SHL 1.
                    // If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
                    uint8_t msb = (cPtr->V[dPtr->x] >> 7) & 0x1;
                    cPtr->V[dPtr->x] = cPtr->V[dPtr->x] * 2;
                    cPtr->V[0xF] = msb;  
                    break;
                }
                
                default: {
                    break;
                }
            }
            break;
        }

        case (0x9): {
            // Skip next instruction if Vx != Vy.
            // The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
            if (cPtr->V[dPtr->x] != cPtr->V[dPtr->x]) {
                cPtr->pc += 2;
            }
            break;
        }

        case (0xA): {
            // Annn - LD I, addr
            // Set I = nnn.The value of register I is set to nnn.
            cPtr->I = dPtr->nnn;

            break;
        }

        case (0xB):
            break;

        case (0xC):
            break;

        case (0xD):
            break;

        case (0xE):
            break;

        case (0xF):
            break;

        default:
            break;
        
    }
    return 1;
};


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