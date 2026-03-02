#include "chip8.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>


void initializeCpu(struct chip8CPU *cpuPtr) {
    /*
    0 - srand(time(NULL));
    Why?
    Becuase it is used to generate a different pattern of generating random numbers as required in instruction cxkk.

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

    srand(time(NULL));
    
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
                    if (cPtr->sp == 0) {
                        printf("Stack Underflow\n");
                        return 0;
                    }                    
                    cPtr->sp -= 1;
                    cPtr->pc = cPtr->stack[cPtr->sp];
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

            cPtr->stack[cPtr->sp] = cPtr->pc;
            cPtr->sp += 1;
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
            if (cPtr->V[dPtr->x] != cPtr->V[dPtr->y]) {
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

        case (0xB): {
            // Bnnn - JP V0, addr -> Jump to location nnn + V0.
            // The program counter is set to nnn plus the value of V0.
            cPtr->pc = dPtr->nnn + cPtr->V[0x0];

            break;
        }

        case (0xC): {
            // Cxkk - RND Vx, byte -> Set Vx = random byte AND kk.
            // The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk. The results are stored in Vx.
            uint8_t randomNumber = rand() % 256;
            cPtr->V[dPtr->x] = randomNumber & dPtr->kk;
            break;
        }

        case (0xD): {
            // Dxyn - DRW Vx, Vy, nibble
            // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = 1, if collision occurs, else VF = 0.
            /*
            The interpreter reads n bytes from memory, starting at the address stored
            in I. These bytes are then displayed as sprites on screen at coordinates
            (Vx, Vy). Sprites are XORed onto the existing screen. If this causes any
            pixels to be erased, VF is set to 1, otherwise it is set to 0. If the
            sprite is positioned so part of it is outside the coordinates of the
            display, it wraps around to the opposite side of the screen. See
            instruction 8xy3 for more information on XOR, and section 2.4, Display,
            for more information on the Chip-8 screen and sprites.
            */

           cPtr->V[0xF] = 0;

            // initialize screen coordinates and add a modulo operator to do a wrap around effect in case the value goes outside the visible range.
           uint8_t xPos = cPtr->V[dPtr->x];
           uint8_t yPos = cPtr->V[dPtr->y];
           
           if (xPos > 64) {
               xPos = xPos % 64;
            }
            if (yPos > 32) {
                yPos = yPos % 32;
            }

            // reads n bytes from memory, starting at the address stored in I.
            for (int row = 0; row < dPtr->n; row++) {
                uint8_t spriteByte = cPtr->ram[cPtr->I + row];

                for (int col = 0; col < 8; col++) {
                    // Extract each bit from left to right
                    uint8_t spritePixel = (spriteByte >> (7 - col)) & 0x1;

                    int displayX = (xPos + col) % 64;
                    int displayY = (yPos + row) % 32;

                    int index = displayY * 64 + displayX;

                    // Collision detection
                    if (spritePixel == 1 && cPtr->display[index] == 1) {
                        cPtr->V[0xF] = 1;
                    }

                    cPtr->display[index] ^= spritePixel;
                }
            }        
            
            break;
        }

        case (0xE): {
            switch (dPtr->kk) {
                case 0x9E: {
                    // Ex9E - SKP Vx: Skip next instruction if key in Vx is pressed
                    uint8_t key = cPtr->V[dPtr->x];
                    if (cPtr->keys[key] != 0) {
                        cPtr->pc += 2;
                    }
                    break;
                }
                case 0xA1: {
                    // ExA1 - SKNP Vx: Skip next instruction if key in Vx is NOT pressed
                    uint8_t key = cPtr->V[dPtr->x];
                    if (cPtr->keys[key] == 0) {
                        cPtr->pc += 2;
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }


        case (0xF): {
            switch (dPtr->kk) {
                case (0x07): {
                    // Fx07 - LD Vx, DT
                    // Set Vx = delay timer value.
                    cPtr->V[dPtr->x] = cPtr->DT;
                    break;
                }
                case (0x0A): {
                    // Fx0A - LD Vx, K
                    // Wait for a key press, store the value of the key in Vx.
                    int keyPressed = 0;
                    for (int i = 0; i < 16; i++) {
                        if (cPtr->keys[i] != 0) {
                            cPtr->V[dPtr->x] = i;
                            keyPressed = 1;
                            // Optionally reset the key state if the game expects it (not always necessary)
                            // cPtr->keys[i] = 0;
                            break;  // Store the first pressed key (adjust if you want to handle multiple)
                        }
                    }
                    if (keyPressed == 0) {
                        // No key pressed: rewind PC to re-execute this instruction next cycle
                        cPtr->pc -= 2;
                    }
                    break;
                }
                case (0x15): {
                    // Fx15 - LD DT, Vx
                    // Set delay timer = Vx.
                    cPtr->DT = cPtr->V[dPtr->x];
                    break;
                }
                case (0x18): {
                    // Fx18 - LD ST, Vx
                    // Set sound timer = Vx.
                    cPtr->ST = cPtr->V[dPtr->x];
                    break;
                }
                case (0x1E): {
                    // Fx1E - ADD I, Vx
                    // Set I = I + Vx.
                    cPtr->I += cPtr->V[dPtr->x];
                    break;
                }
                case (0x29): {
                    // Fx29 - LD F, Vx
                    // Set I = location of sprite for digit Vx.
                    cPtr->I = (cPtr->V[dPtr->x] & 0xF) * 5;
                    break;
                }
                case (0x33): {
                    // Fx33 - LD B, Vx
                    // Store BCD representation of Vx in memory locations I, I+1, and I+2.
                    uint8_t val = cPtr->V[dPtr->x];
                    cPtr->ram[cPtr->I]     = val / 100;
                    cPtr->ram[cPtr->I + 1] = (val / 10) % 10;
                    cPtr->ram[cPtr->I + 2] = val % 10;
                    break;
                }
                case (0x55): {
                    // Fx55 - LD [I], Vx
                    // Store registers V0 through Vx in memory starting at location I.
                    for (int i = 0; i <= dPtr->x; i++) {
                        cPtr->ram[cPtr->I + i] = cPtr->V[i];
                    }
                    // Note: Original CHIP-8 does not increment I (some variants do)
                    break;
                }
                case (0x65): {
                    // Fx65 - LD Vx, [I]
                    // Read registers V0 through Vx from memory starting at location I.
                    for (int i = 0; i <= dPtr->x; i++) {
                        cPtr->V[i] = cPtr->ram[cPtr->I + i];
                    }
                    // Note: Original CHIP-8 does not increment I (some variants do)
                    break;
                }
                default: {
                    // Unknown Fxkk instruction: ignore or log
                    printf("Unknown instruction: 0x%04X\n", (dPtr->opcode << 12) | (dPtr->x << 8) | (dPtr->y << 4) | dPtr->n);
                    break;
                }
            }
            break;
        }

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