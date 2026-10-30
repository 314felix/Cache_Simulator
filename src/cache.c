#include "memory.h"
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <math.h>
#include "cache.h"

cache_t *cache_create(int cacheSize, int lineSize, int nWays, int writePolicy, int inclusivePolicy, int replacementPolicy) {
    /*Creates a new cache*/

    if (cacheSize <= 0 || lineSize <= 0 || nWays <= 0) {
        printf("\033[31mERROR: Cache size, line size and number of ways must be grater then zero.\n\n\033[0m");
        exit(EXIT_FAILURE);
    }


    if (inclusivePolicy != 0 && inclusivePolicy != 1) {
        printf("\033[31mERROR: Inclusive policy must be set to either 0 [exclusive] or 1 [inclusive].\n\n\033[0m");
        exit(EXIT_FAILURE);
    }

    if (replacementPolicy != 0 && replacementPolicy != 1 && replacementPolicy != 2) {
        printf("\033[31mERROR: Replacement policy must be set to either 0 [random], 1 [LRU] or 2 [temporal/spatial].\n\n\033[0m");
        exit(EXIT_FAILURE);
    }

    if (writePolicy != 0 && writePolicy != 1) {
        printf("\033[31mERROR: Write policy must be set to either 0 [write-back] or 1 [write-through].\n\n\033[0m");
        exit(EXIT_FAILURE);
    }

    if (cacheSize % lineSize != 0) {
        printf("\033[31mERROR: Cache size must be a multiple of the line size.\n\n\033[0m");
        exit(EXIT_FAILURE);
    }

    int nBlocks = cacheSize/lineSize;

    if (nBlocks % nWays != 0) {
        printf("\033[31mERROR: Number of cache blocks must be divisible by the number of ways.\n\n\033[0m");
        exit(EXIT_FAILURE);
    }

    int nSets = nBlocks / nWays;

    if (floor(log2(lineSize)) != log2(lineSize) || (floor(log2(nSets)) != log2(nSets)))  {
        printf("\033[31mERROR: Cache line size and number of sets must both be powers of two.\n\n\033[0m");
        exit(EXIT_FAILURE);
    }

    cache_t *cache = malloc(sizeof(cache_t));

    if (cache == NULL) {
        printf("\033[31mERROR: Failed to allocate memory for the cache.\n\n\033[0m");
        exit(EXIT_FAILURE);
    }

    cache->sets = calloc(nSets, sizeof(set_t));

    if (cache->sets == NULL) {
        free(cache);
        printf("\033[31mERROR: Failed to allocate memory for the cache sets.\n\n\033[0m");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < nSets; i++) {
        cache->sets[i].nBlocks = nWays;
        cache->sets[i].index   = i;

        cache->sets[i].blocks = calloc(nWays, sizeof(block_t));

        if (cache->sets[i].blocks == NULL) {
            for (int x = 0; x < i; x++) {
                free(cache->sets[x].blocks);
            }
            free(cache->sets);
            free(cache);
            printf("\033[31mERROR: Failed to allocate memory for the set blocks.\n\n\033[0m");
            exit(EXIT_FAILURE);
        }

        for (int y = 0; y < nWays; y++) {
            cache->sets[i].blocks[y].lastUsed = 0;
            cache->sets[i].blocks[y].index    = y;
            cache->sets[i].blocks[y].valid    = 0;
            cache->sets[i].blocks[y].tag      = 0;
            cache->sets[i].blocks[y].dirty    = 0;
        }
    }

    cache->replacementPolicy = replacementPolicy;
    cache->inclusivePolicy = inclusivePolicy;
    cache->writePolicy = writePolicy;
    cache->lineSize = lineSize;
    cache->nBlocks = nBlocks;
    cache->nSets = nSets;
    cache->hits = 0;
    cache->misses = 0;
    cache->writes = 0;

    return cache;
}

void cache_destroy(cache_t *cache) {
    /*Removes cache from memory*/

    if (cache == NULL) {
        printf("\033[031mERROR destroying cache, cache not found\033[0m\n");
        return;
    }

    for (int i = 0; i < cache->nSets; i++) {
        free(cache->sets[i].blocks);
    }

    free(cache->sets);
    free(cache);
}

void print_cache_parameters(cache_t *cache, char *name) {
    /* Print the cache parameters in the console */
    printf("\033[092m%s - parameters: \033[0m\n", name);

    printf("\033[032m - Cache associativity: \033[0m");

    if (cache->nBlocks / cache->nSets == 1) {
        printf("\033[036mDirect mapped\033[0m\n\n");
    } else if (cache->nSets == 1) {
        printf("\033[036mFully associative\033[0m\n");
    } else {
        printf("\033[036m%i-way\033[0m\n", cache->nBlocks / cache->nSets);
    }

    printf("\033[032m - Replacement policy:  \033[0m");

    if (cache->replacementPolicy == 0) {
        printf("\033[036mrandom\033[0m\n");
    } else if (cache->replacementPolicy == 1) {
        printf("\033[036mLRU\033[0m\n");
    } else {
        printf("\033[036mtemporal/spatial\033[0m\n");
    }

    printf("\033[032m - Write policy:        \033[0m");

    if (cache->writePolicy == 0) {
        printf("\033[036mwrite-back \033[0m\n");
    } else {
        printf("\033[036mwrite-through \033[0m\n");
    }

    printf("\033[032m - Cache size:         \033[036m %.1f KB\033[0m\n", (float)(cache->nBlocks * cache->lineSize)/1024);
    printf("\033[032m - Line size:          \033[036m %i bytes\033[0m\n", (cache->lineSize));
    printf("\033[032m - Blocks:             \033[036m %i\033[0m\n", (cache->nBlocks));
    printf("\033[032m - Sets:               \033[036m %i\033[0m\n\n", (cache->nSets));
}

void print_cache_statistics(cache_t *cache, char *name) {
    /* Print the cache statistics in the console */
    printf("\033[036m%s - statistics: \033[0m\n", name);
    printf("\033[094m - Cache hits:   \033[092m %i\033[0m\n", (cache->hits));
    printf("\033[094m - Cache misses: \033[091m %i\033[0m\n", (cache->misses));
    printf("\033[094m - Cache writes: \033[091m %i\033[0m\n\n", (cache->writes));
}

uint32_t cache_get_index(cache_t *cache, uint32_t address) {
    /* Calculates the cache index from the address */

    int index_size = log2(cache->nSets);
    int tag_size   = 32 - (index_size + (log2(cache->lineSize) - 2) + 2);
    uint32_t tag   = (address >> (32 - tag_size));
    uint32_t index = (address >> (32 - tag_size - index_size) & (cache->nSets - 1));

    return index;
}

uint32_t cache_get_tag(cache_t *cache, uint32_t address) {
    /* Calculates the cache tag from the address */

    int index_size = log2(cache->nSets);
    int tag_size   = 32 - (index_size + (log2(cache->lineSize) - 2) + 2);
    uint32_t tag   = (address >> (32 - tag_size));

    return tag;
}

set_t *cache_get_set(cache_t *cache, uint32_t address) {
    /* Search for valid set in the cache, based on the address */

    int index = cache_get_index(cache, address);

    set_t *set = &cache->sets[index];

    if (set == NULL) {
        return NULL;
    } else {
        return set;
    }
}

block_t *cache_get_block(cache_t *cache, uint32_t address) {
    /* Search for block set in the cache, based on the address */

    int index = cache_get_index(cache, address);
    int tag   = cache_get_tag(cache, address);

    set_t *set = cache_get_set(cache, address);

    if (set == NULL) {
        return NULL;
    }

    for (int i = 0; i < set->nBlocks; i++) {
        if (set->blocks[i].tag == tag && set->blocks[i].valid == 1) {
            return &set->blocks[i];;
        }
    }

    return NULL;

}

block_t *cache_get_replacement_block(cache_t *cache, uint32_t address) {
    /* Search for a replacement block in the cache, based on the replacement policy */

    set_t *set = cache_get_set(cache, address);

    // Search for available invalid blocks
    for (int i = 0; i < set->nBlocks; i++) {
        if (set->blocks[i].valid == 0) {
            return &set->blocks[i];
        }
    }

    // Overweight cases controlled by the replacement policy
    if (cache->replacementPolicy == 0) {
        // random - replacement policy

        int random_index = rand() % set->nBlocks;
        return &set->blocks[random_index];

    } else if (cache->replacementPolicy == 1) {
        // LRU -  replacement policy

        int LRU = 0;
        for (int i = 0; i < set->nBlocks; i++) {
            if (set->blocks[i].lastUsed > set->blocks[LRU].lastUsed) {
                LRU = i;
            }
        }

        return &set->blocks[LRU];

    } else {
        // temporal/spatial -  replacement policy
        /* Not implimented yet... */
        printf("\033[31mERROR: the temporal/spatial replacement policy is not yet defined\n\033[0m");
        return NULL;
    }

    return NULL;
}

void cache_update_lru(cache_t *cache, uint32_t address) {
    /* Increase the LRU of all blocks, except the block for the current address.  */

    int index = cache_get_index(cache, address);
    int tag   = cache_get_tag(cache, address);

    set_t *set = cache_get_set(cache, address);

    if (set == NULL) {
        return;
    }

    for (int i = 0; i < set->nBlocks; i++) {

        block_t *block = &set->blocks[i];

        if (block->valid) {

            if (block->tag == tag) {
                block->lastUsed = 0;
            } else {
                block->lastUsed++;
            }
        }
    }

    return;
}

void cache_set_block(cache_t *cache, uint32_t address, block_t *block) {
    /* Sets the parameter of specified block according with the cache */

    int tag = cache_get_tag(cache, address);

    block->lastUsed = 0;
    block->valid = 1;
    block->tag = tag;
}