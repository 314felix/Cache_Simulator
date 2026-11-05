/** @file memory.c
 *  @brief Implements starting point for a memory hierarchy with caching and
 * RAM.
 *  @see memory.h
 */

#include "memory.h"

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "cache.h"

static unsigned long instr_count;

// global cache
static cache_t *l1_instruction_cache = NULL;
static cache_t *l1_data_cache   = NULL;
static cache_t *l2_unfied_cache = NULL;

void memory_init(void)
{
  /* Initialize memory subsystem here. */
  printf("Initializing memory subsystem\n\n");

  /* L1 Instruction cache */
  l1_instruction_cache = cache_create(
    // Size of the cache (bytes)
    4 * 1024,
    // Size of the line size (bytes)
    16,
    // n-way associativity
    2,
    // Write Policy: [0] write-back, [1] write-through
    0,
    // Replacement Policy: [0] random, [1] LRU, [2] temporal/spatial
    1
  );

  /* L1 Data cache */
  l1_data_cache = cache_create(
    // Size of the cache (bytes)
    32 * 1024,
    // Size of the line size (bytes)
    64,
    // n-way associativity
    8,
    // Write Policy: [0] write-back, [1] write-through
    0,
    // Replacement Policy: [0] random, [1] LRU, [2] temporal/spatial
    1
  );

  /* L2 Unified cache */
  l2_unfied_cache = cache_create(
    // Size of the cache (bytes)
    512 * 1024,
    // Size of the line size (bytes)
    64,
    // n-way associativity
    8,
    // Write Policy: [0] write-back, [1] write-through
    0,
    // Replacement Policy: [0] random, [1] LRU, [2] temporal/spatial
    1
  );

  // print cache parameters
  print_cache_parameters(l1_instruction_cache, "L1 Instruction cache");
  print_cache_parameters(l1_data_cache, "L1 Data cache");
  print_cache_parameters(l2_unfied_cache, "L2 Unified cache");

  instr_count = 0;
}

void memory_fetch(uint32_t* address, data_t *data)
{
  //printf("memory: fetch %p\n", address);
  if (data)
    *data = (data_t)0;

  instr_count++;

  // Converts the double address to uint32_t
  uint32_t adr = (uint32_t)(uintptr_t)address;
  if (cache_get_block(l1_instruction_cache, adr)){
    l1_instruction_cache->hits++;
    cache_update_lru(l1_instruction_cache, adr);
    return;
  } 
  
  l1_instruction_cache->misses++;
  block_t *replaced_block = cache_get_replacement_block(l1_instruction_cache, adr);
  cache_set_block(l1_instruction_cache, adr, replaced_block);
  cache_update_lru(l1_instruction_cache, adr);

  if (cache_get_block(l2_unfied_cache, adr)){
    l2_unfied_cache->hits++;
    cache_update_lru(l2_unfied_cache, adr);
    return;
  }

  l2_unfied_cache->misses++;
  block_t *replaced_block_l2 = cache_get_replacement_block(l2_unfied_cache, adr);
  cache_set_block(l2_unfied_cache, adr, replaced_block_l2);
  cache_update_lru(l2_unfied_cache, adr);

}

void memory_read(double* address, data_t *data)
{
  if (data)
    *data = (data_t)0;

  instr_count++;

  // Converts the double address to uint32_t
  uint32_t adr = (uint32_t)(uintptr_t)address;



  if (cache_get_block(l1_data_cache, adr)){
    l1_data_cache->hits++;
    cache_update_lru(l1_data_cache, adr);
    return;
  } 
  
  l1_data_cache->misses++;
  block_t *replacement_block = cache_get_replacement_block(l1_data_cache, adr);
  if (replacement_block->dirty == 1) {
    
    // Replacement block is dirty, lower levels must be updated
    uint32_t tag   = replacement_block->tag;
    uint32_t index = cache_get_index(l1_data_cache, adr);
    
    uint32_t old_adr = cache_reconstruct_address(l1_data_cache, index, tag);
    
    block_t *l2_block = cache_get_block(l2_unfied_cache, old_adr);
    if (l2_block != NULL) {
    // L2 unfied cache - Cache hit
    
    l2_unfied_cache->hits++;
    /*The block is not actually written according with the
    simplifications. But the action is registered as a write.
    Writing to level 3 memory is not simulated. */
    l2_unfied_cache->writes++;
          
    
    cache_update_lru(l2_unfied_cache, old_adr);
    if (l2_unfied_cache->writePolicy == 0) {
      l2_block->dirty = 1;
      
    }
    // L2 unfied cache - Cache miss
  } else {
    l2_unfied_cache->writes++;
    l2_unfied_cache->misses++;
    
    block_t *l2_replacement_block = cache_get_replacement_block(l2_unfied_cache, old_adr);
    cache_set_block(l2_unfied_cache, old_adr, l2_replacement_block);
    cache_update_lru(l2_unfied_cache, old_adr);

    if (l2_unfied_cache->writePolicy == 0) {
      l2_replacement_block->dirty = 1;
    }
  }
}
        
     

  
  cache_set_block(l1_data_cache, adr, replacement_block);
  cache_update_lru(l1_data_cache, adr);

  block_t *l2_block = cache_get_block(l2_unfied_cache, adr);
  if (l2_block != NULL) {
    l2_unfied_cache->hits++;
    cache_update_lru(l2_unfied_cache, adr);
    return;
  }

  l2_unfied_cache->misses++;
  block_t *replaced_block_l2 = cache_get_replacement_block(l2_unfied_cache, adr);
  cache_set_block(l2_unfied_cache, adr, replaced_block_l2);
  cache_update_lru(l2_unfied_cache, adr);

}

void memory_write(double* address, data_t *data)
{
  instr_count++;

  // Converts the double address to uint32_t
  uint32_t adr = (uint32_t)(uintptr_t)address;

  if (l1_data_cache->writePolicy == 1) {
    // WritePolicy: write-through
    block_t *l1_block = cache_get_block(l1_data_cache, adr);

    if (l1_block != NULL) {
      // L1 data cache - Cache hit
      l1_data_cache->hits++;
      l1_data_cache->writes++;
      cache_update_lru(l1_data_cache, adr);

    } else {
      // L1 data cache - Cache miss
      l1_data_cache->misses++;
      l1_data_cache->writes++;

      block_t *replacement_block = cache_get_replacement_block(l1_data_cache, adr);
      cache_set_block(l1_data_cache, adr, replacement_block);
      cache_update_lru(l1_data_cache, adr);
    }

    block_t *l2_block = cache_get_block(l2_unfied_cache, adr);

    if (l2_block != NULL) {
      // L2 unfied cache - Cache hit
      l2_unfied_cache->hits++;
      l2_unfied_cache->writes++;
      cache_update_lru(l2_unfied_cache, adr);

      if (l2_unfied_cache->writePolicy == 0) {
        l2_block->dirty = 1;
      }

    } else {
      // LL2 unfied cache - Cache miss
      l2_unfied_cache->misses++;
      l2_unfied_cache->writes++;

      block_t *replacement_block = cache_get_replacement_block(l2_unfied_cache, adr);
      cache_set_block(l2_unfied_cache, adr, replacement_block);
      cache_update_lru(l2_unfied_cache, adr);

      if (l2_unfied_cache->writePolicy == 0) {
        replacement_block->dirty = 1;
      }
    }
    
  } else {
    // WritePolicy: write-back
    block_t *block = cache_get_block(l1_data_cache, adr);

    if (block != NULL) {
      // L1 data cache - Cache hit
      l1_data_cache->hits++;
      l1_data_cache->writes++;
      block->dirty = 1;
      cache_update_lru(l1_data_cache, adr);

    } else {
      // L1 data cache - Cache miss
      l1_data_cache->misses++;
      block_t *replacement_block = cache_get_replacement_block(l1_data_cache, adr);

      if (replacement_block->dirty == 1) {
        // Replacement block is dirty, lower levels must be updated

        uint32_t index = cache_get_index(l1_data_cache, adr);
        uint32_t tag   = replacement_block->tag;
        uint32_t old_adr = cache_reconstruct_address(l1_data_cache, index, tag);

        block_t *l2_block = cache_get_block(l2_unfied_cache, old_adr);

        if (l2_block != NULL) {
          // L2 unfied cache - Cache hit
          l2_unfied_cache->hits++;

          /*The block is not actually written according with the
          simplifications. But the action is registered as a write.
          Writing to level 3 memory is not simulated. */

          l2_unfied_cache->writes++;
          cache_update_lru(l2_unfied_cache, old_adr);

          if (l2_unfied_cache->writePolicy == 0) {
            l2_block->dirty = 1;
          }

        } else {
          // L2 unfied cache - Cache miss
          l2_unfied_cache->misses++;
          l2_unfied_cache->writes++;

          block_t *l2_replacement_block = cache_get_replacement_block(l2_unfied_cache, old_adr);
          cache_set_block(l2_unfied_cache, old_adr, l2_replacement_block);
          cache_update_lru(l2_unfied_cache, old_adr);

          if (l2_unfied_cache->writePolicy == 0) {
            l2_replacement_block->dirty = 1;
          }
        }
      }

      // Read data from lower memory into the cache block
      block_t *l2_block = cache_get_block(l2_unfied_cache, adr);

      if (l2_block != NULL) {
        // L2 unfied cache - Cache hit
        l2_unfied_cache->hits++;
        cache_update_lru(l2_unfied_cache, adr);

      } else {
        // L2 unfied cache - Cache miss
        l2_unfied_cache->misses++;
        block_t *l2_replacement_block = cache_get_replacement_block(l2_unfied_cache, adr);
        cache_set_block(l2_unfied_cache, adr, l2_replacement_block);
        cache_update_lru(l2_unfied_cache, adr);
      }

      // write to L1 data cache
      l1_data_cache->writes++;
      cache_set_block(l1_data_cache, adr, replacement_block);
      replacement_block->dirty = 1;
      cache_update_lru(l1_data_cache, adr);

    }
  }
}

void memory_finish(void)
{
  fprintf(stdout, "Executed %lu instructions.\n\n", instr_count);

  // print the statistics
  print_cache_statistics(l1_instruction_cache, "L1 Instruction cache");
  print_cache_statistics(l1_data_cache, "L1 Data cache");
  print_cache_statistics(l2_unfied_cache, "L2 Unified cache");

  record_cache_results(l1_instruction_cache, "L1 Instruction cache");
  record_cache_results(l1_data_cache, "L1 Data cache");
  record_cache_results(l2_unfied_cache, "L2 Unified cache");

  // Deinitialize memory subsystem here
  cache_destroy(l1_instruction_cache);
  cache_destroy(l1_data_cache);
  cache_destroy(l2_unfied_cache);
}
