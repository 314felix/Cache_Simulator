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
    32 * 1024,
    // Size of the line size (bytes)
    64,
    // n-way associativity
    4,
    // Write Policy: [0] write-back, [1] write-through
    0,
    // Inclusive Policy: [0] exclusive, [1] inclusive
    0,
    // Replacement Policy: [0] random, [1] LRU, [2] temporal/spatial
    0
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
    // Inclusive Policy: [0] exclusive, [1] inclusive
    1,
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
    // Inclusive Policy: [0] exclusive, [1] inclusive
    1,
    // Replacement Policy: [0] random, [1] LRU, [2] temporal/spatial
    0
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

  // Instruction Memory

}

void memory_read(double* address, data_t *data)
{
  //printf("memory: read %p\n", address);
  if (data)
    *data = (data_t)0;

  instr_count++;

  // Converts the double address to uint32_t
  uint32_t adr = (uint32_t)(uintptr_t)address;

  // Instruction Memory

}

void memory_write(double* address, data_t *data)
{
  //printf("memory: write %p\n", address);

  instr_count++;

  // Converts the double address to uint32_t
  uint32_t adr = (uint32_t)(uintptr_t)address;

  // Data Memory

}

void memory_finish(void)
{
  fprintf(stdout, "Executed %lu instructions.\n\n", instr_count);

  // print the statistics
  print_cache_statistics(l1_instruction_cache, "L1 Instruction cache");
  print_cache_statistics(l1_data_cache, "L1 Data cache");
  print_cache_statistics(l2_unfied_cache, "L2 Unified cache");

  // Deinitialize memory subsystem here
  cache_destroy(l1_instruction_cache);
  cache_destroy(l1_data_cache);
  cache_destroy(l2_unfied_cache);
}
