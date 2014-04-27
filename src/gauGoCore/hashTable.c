/**
 * @file hashTable.c
 * @brief HashTable implementation
 *
 **/

#include "hashTable.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

void HashTable_initialize(HashTable* table, int maskSizeBits, int bucketSizeBytes)
{
  // Set properties
  table->maskSizeBits = maskSizeBits;
  table->mask = (1 << maskSizeBits) - 1;
  table->bucketSizeBytes = bucketSizeBytes;
  table->entriesNum = 0;
  table->firstKeyCollisions = 0;
  
  // Allocate memory
  table->memory = calloc( 1 << maskSizeBits, sizeof(HashKey) + bucketSizeBytes );
}

void HashTable_delete(HashTable* table)
{
  free( table->memory );
  table->memory = NULL;
}

void* HashTable_insert(HashTable* table, HashKey* key, void* data)
{
  // First probe
  HashKey* probe = (HashKey*)((char*)table->memory + (key->key1 & table->mask) * (sizeof(HashKey) + table->bucketSizeBytes));

  // Empty slot found
  if( probe->key1 == 0 && probe->key2 == 0 ){
    *(probe) = *key;
    memcpy(((char*)probe) + sizeof(HashKey), data, table->bucketSizeBytes);
  }
  else{
    // First key collision
    table->firstKeyCollisions++;

    // Search at intervals of key2
    int i=1;
    do {
      probe = (HashKey*)((char*)table->memory + ((key->key1 + i*key->key2) & table->mask) * (sizeof(HashKey) + table->bucketSizeBytes));
      i++;
    } 
    while( probe->key1 != 0 || probe->key2 != 0 );

    *(probe) = *key;
    memcpy(((char*)probe) + sizeof(HashKey), data, table->bucketSizeBytes);
  }
  
  table->entriesNum++;

  return ((char*)probe) + sizeof(HashKey);
}

/**
 * @brief Retrieves data stored in the hash table with specified key.
 * 
 * @param table The table
 * @param key   The key which maps to stored data
 * @return The pointer to stored data if key is stored into the hash table, NULL otherwise
 **/
void* HashTable_retrieve(HashTable* table, HashKey* key)
{
  // Search at intervals of key2 until match or empty slot
  int i=0;
  HashKey* probe;

  while( 1 ){
    probe = (HashKey*)((char*)table->memory + ((key->key1 + i*key->key2) & table->mask) * (sizeof(HashKey) + table->bucketSizeBytes));
    if( probe->key1==0 && probe->key2==0 ) return NULL;
    if( probe->key1==key->key1 && probe->key2==key->key2 ) return ((char*)probe) + sizeof(HashKey);

    i++;
  }
}
