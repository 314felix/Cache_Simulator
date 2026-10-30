/** @file cache.h
 *  @brief a collection of cache functions
 *  @see cache.c
 */

#include <stdint.h>

/* block_t represent a cache block which make up the cache.
The data is not stored according with the simplification in README
*/
typedef struct block block_t;
struct block {
    int lastUsed;
    int index;
    int valid;
    int tag;
    int dirty;
};

/* set_t represent a cache set.
The set is in turn made up from blocks 
*/
typedef struct set set_t;
struct set {
    block_t *blocks;
    int nBlocks;
    int index;
};


/* cache_t represent the cache made up of blocks.
It keeps track of hits, misses and writes
*/
typedef struct cache cache_t;
struct cache {
    set_t *sets;
    int replacementPolicy;
    int inclusivePolicy;
    int writePolicy;
    int lineSize;
    int nBlocks;
    int nSets;      
    int hits;
    int misses;
    int writes;
};

/** Creates a cache from different parameter 
 * 
 *  @param[in]  cacheSize Size of the cache
 *  @param[in]  lineSize Size of the line size
 *  @param[in]  nWays n-way associativity
 *  @param[in]  writePolicy [0] write-back, [1] write-through
 *  @param[in]  inclusivePolicy [0] exclusive, [1] inclusive
 *  @param[in]  replacementPolicy [0] random, [1] LRU, [2] temporal/spatial
 *  @param[out] cache Newly created cache
 */
cache_t *cache_create(int cacheSize, int lineSize, int nWays, int writePolicy, int inclusivePolicy, int replacementPolicy);

/** Removes the cache from memory
 * 
 *  @param[in] cache Cache to be removed
 */
void cache_destroy(cache_t *cache);

/** Print the cache parameters in the console
 * 
 *  @param[in] cache Cache to be printed
 */
void print_cache_parameters(cache_t *cache, char *name);

/** Print the cache statistics in the console
 * 
 *  @param[in] cache Cache to be printed
 */
void print_cache_statistics(cache_t *cache, char *name);

/** Calculates the cache index from the address
 * 
 * @param[in] cache Cach the index is created for
 * @param[in] address Address the index is calculated from
 * @param[out] index return index
*/
uint32_t cache_get_index(cache_t *cache, uint32_t address);

/** Calculates the cache tag from the address
 * 
 * @param[in] cache Cach the tag is created for
 * @param[in] address Address the tag is calculated from
 * @param[out] index return tag
 */
uint32_t cache_get_tag(cache_t *cache, uint32_t address);

/** Search for a set in the cache, based on the address
 * 
 * @param[in] cache Cache in which the set is searched for
 * @param[in] address Address used to calculate the set index
 * @param[out] set return set, NULL on not found
*/
set_t *cache_get_set(cache_t *cache, uint32_t address);

/** Search for a block in the cache, based on the address
 * 
 * @param[in] cache Cache in which the block is searched for
 * @param[in] address Address used to calculate the block tag
 * @param[out] block return block, NULL on not found
 */
block_t *cache_get_block(cache_t *cache, uint32_t address);

/** Search for a replacement block in the cache, based on the replacement policy
 * 
 * @param[in] cache Cache in which the replacement block is searched for
 * @param[in] address Address used to calculate the block tag
 * @param[out] replacement_block return replacement block, NULL on not found
 */
block_t *cache_get_replacement_block(cache_t *cache, uint32_t address);

/** Increase the LRU of all blocks, except the block for the current address.
 * 
 * @param[in] cache Cache in which the LRU block set is updated
 * @param[in] address Address used to calculate current block
 */
void cache_update_lru(cache_t *cache, uint32_t address);

/** Sets the parameter of specified block according with the cache
 * 
 * @param[in] cache Cache in which the block parameters are updated
 * @param[in] address Address used to calculate block parameters
 * @param[in] block Block to be updated
 */
void cache_set_block(cache_t *cache, uint32_t address, block_t *block);