/**
 * @file  hashTable.h
 * @brief Provides an implementation of a double-hashing open-addressed hash table.
 *
 * The hash table's size and bucket size can be specified by the user at creation time.
 * Element retrival and storage need a double hash value to be specified by the user.
 * Performance of every operation is strictly dependant on the load factor, and in 
 * particular for load factors near to 1.0 performance drops badly.
 *  
 **/
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdio.h>

/**
 * @brief Hash key to the entries in the hash table.
 * Every table entry must map to 2 different 32-bit keys.
 * It is assumed that the combination of (key1, key2) is unique.
 **/
typedef struct HashKey
{
  unsigned long long key1;
  unsigned long long key2;
} HashKey;


/**
 * @brief Data type of open-addressed double-hashing hash table.
 **/
typedef struct HashTable
{
  /** Size in bits of the table's mask (table actual size is 2^sizeBits * bucketSize)*/
  int maskSizeBits;
  /** Table 32-bit mask */
  int mask;

  /** Size of one bucket in bytes */
  int bucketSizeBytes;

  /** Allocated memory */
  void* memory;


  /** Usage/Collision info */
  int entriesNum;
  int firstKeyCollisions;

} HashTable;


/**
 * @brief Initializes a HashTable structure with specified mask size and bucket size.
 *
 * @param board table The hash table to initialize
 * @param maskSizeBits The bit length of the mask.  This determines the hash table size
 * @param bucketSizeBytes The size in bytes of data that will be store in a bucket
 **/
void HashTable_initialize(HashTable* table, int maskSizeBits, int bucketSizeBytes);

/**
 * @brief Release resources allocated with the hash table.
 * 
 * @param table The table to release.  After deletion, the table must not be used.
 **/
void HashTable_delete(HashTable* table);

/**
 * @brief Insert an entry into the hashTable.
 *
 * @param table The table
 * @param key   The hash key (double-hash) of the data to be stored
 * @param data  The data to be stored in the table.  The length of the data
 * to be stored must match the size of one bucket that was specified at table 
 * initialization
 *
 * @return The pointer to newly stored data
 **/
void* HashTable_insert(HashTable* table, HashKey* key, void* data);

/**
 * @brief Retrieves data stored in the hash table with specified key.
 * 
 * @param table The table
 * @param key   The key which maps to stored data
 * @return The pointer to stored data if key is stored into the hash table, NULL otherwise
 **/
void* HashTable_retrieve(HashTable* table, HashKey* key);

/**
 * @brief Test 2 hash keys for equality
 *
 * @param key1 First key
 * @param key2 Second key
 * @param return Equality
 **/
int HashKey_compare(HashKey* key1, HashKey* key2);

#endif
