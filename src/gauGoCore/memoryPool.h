/**
 * @file  memoryPool.h
 * @brief A self-growing pre-allocated fast access memory pool
 *  
 **/
#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

/**
 * @brief Memory pool for fast-allocations
 **/
typedef struct MemoryPool
{
  // Pool memory
  void* memory;
  // Pool end
  void* end;

  // One element size in bytes
  int elementSize;
  
  // Next available block of memory
  void* nextAvailable;

} MemoryPool;

/**
 * @brief Initialize a memory pool for the specified element
 * number and size
 *
 * @param pool The pool
 * @param elementNum Maximum number of elements that the pool 
 * will accomodate
 * @param elementSize The size in bytes of one element
 * @return 1-success 0-out of memory
 **/
int MemoryPool_initialize( MemoryPool* pool, int elementNum, int elementSize );

/**
 * @brief Release the memory allocated for the specified pool
 * The pool and all memory allocated with the pool becomes unusable.
 *
 * @param pool The pool
 **/
void MemoryPool_delete( MemoryPool* pool );

/**
 * @brief Allocates a new element. This is a guaranteed constant-time op
 *
 * @param pool The pool
 * @return The newly allocated memory, or NULL if no more memory is available
 **/
void* MemoryPool_allocate( MemoryPool* pool );

#endif
