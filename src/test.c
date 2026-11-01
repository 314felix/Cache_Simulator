#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "memory.h"

#define N 45  // Matrix size
#define CLS 64  // Cache Line Size
#define SM (CLS/sizeof(double))  // Nb of double in each cache line

#define BENCHMARK       NAIVE


int main(int argc, char *argv[])
{

    memory_init();
    static uint32_t instruction_num[64];
    for (int i = 0; i < 16; i++) {
        memory_fetch(instruction_num+4*i, NULL); 
        memory_fetch(instruction_num+4, NULL); 
        memory_fetch(instruction_num+8, NULL); 
        memory_fetch(instruction_num+12, NULL); 
        memory_fetch(instruction_num+16, NULL); 
        memory_fetch(instruction_num+20, NULL); 
        memory_fetch(instruction_num+24, NULL); 
    }
    memory_finish(); /* Deinitialize the memory subsystem */
    printf("Instruction number: %p\n", instruction_num+5);
    return 0;
    
}
