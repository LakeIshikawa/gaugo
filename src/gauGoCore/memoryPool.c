/**
 * @file uctTree.c
 * @brief UCT tree implementation
 *
 **/
#include "uctTree.h"
#include "crash.h"

#include <stdlib.h>

int MemoryPool_initialize( MemoryPool* pool, int elementNum, int elementSize )
{
  pool->elementSize = elementSize;
  pool->memory = calloc( elementNum, elementSize );
  pool->nextAvailable = pool->memory;
  pool->end = ((unsigned char*)pool->memory) + (elementNum*elementSize);

  return pool->memory != NULL;
}

void MemoryPool_delete( MemoryPool* pool )
{
  free( pool->memory );
}

void* MemoryPool_allocate( MemoryPool* pool )
{
  if( pool->nextAvailable >= pool->end ) return NULL;
  
  // Allocate one element
  void* el = pool->nextAvailable;
  pool->nextAvailable = 
    ((unsigned char*)pool->nextAvailable) 
    + pool->elementSize;

  return el;
}
