#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "memory.h"

#define N 128  // Matrix size
#define CLS 64  // Cache Line Size
#define SM (CLS/sizeof(double))  // Nb of double in each cache line

#define BLOCK_SIZE 64
#define L1_STRIDE 512  // 512 doubles = 4096 bytes
#define L2_STRIDE 8192  // Doubles: 8192 * 8 = 65536 bytes


#define BENCHMARK       NAIVE

int test_replacement(double* c);
int test_replacement_l2(double *c);


int main(int argc, char *argv[])
{

    memory_init();
    // static uint32_t instruction_num[64];
    // for (int i = 0; i < 16; i++) {
    //     memory_fetch(instruction_num+4*i, NULL); 
    //     memory_fetch(instruction_num+4, NULL); 
    //     memory_fetch(instruction_num+8, NULL); 
    //     memory_fetch(instruction_num+12, NULL); 
    //     memory_fetch(instruction_num+16, NULL); 
    //     memory_fetch(instruction_num+20, NULL); 
    //     memory_fetch(instruction_num+24, NULL); 
    // }

    
    int i = 0;
    int j;
    double* c = malloc(N*N * sizeof(double));
    // Assume c points to a valid, initialized array of doubles.

    // for (int line = 0; line < 20; line++)
    // {
    //     // Each cache line holds 8 doubles (64 bytes).
    //     j = line * 8;

    //     // First access: miss
    //     memory_read(c + N*i + j, NULL);

    //     // Second access: hit
    //     memory_read(c + N*i + j, NULL);

    //     // Third access: hit
    //     memory_read(c + N*i + j, NULL);
    // }
    test_replacement_l2(c);

    memory_finish(); /* Deinitialize the memory subsystem */
    return 0;
    
}

int test_replacement(double* c) {
    // Assume c points to a valid array of at least 4097 doubles.
    // The L1 data cache must start empty.

    // Fill one cache set with 8 blocks.
    for (int k = 0; k < 8; k++)
    {
        memory_read(c + k * L1_STRIDE, NULL);
    }

    // Block 0 is now the most recently used block.
    memory_read(c + 0 * L1_STRIDE, NULL);

    // Access a 9th block that maps to the same cache set.
    // This evicts block 1, the least recently used block.
    memory_read(c + 8 * L1_STRIDE, NULL);

    // Block 0 is still in the cache.
    memory_read(c + 0 * L1_STRIDE, NULL);

    // Block 1 was evicted, so this access misses.
    memory_read(c + 1 * L1_STRIDE, NULL);

    return 0;
}


int test_replacement_l2(double *c)
{

    for (int k = 0; k < 9; k++)
    {
        memory_read(c + k * L1_STRIDE, NULL);
    }

    // Block 0 was evicted from L1 but remains in L2.
    memory_read(c, NULL);





    for (int k = 1; k <= 8; k++)
    {
        memory_read(c + k * L2_STRIDE, NULL);
    }

    // Block 0 should now be evicted from L2.
    memory_read(c, NULL);

    // Expected:
    // L1: 19 miss
    // L1: 0 hit
    // L2: 18 miss
    // L2: 1 hit
    return 0;
}