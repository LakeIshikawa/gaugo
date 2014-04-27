/**
 * @file board_zobrist.c
 * @brief Board zobrist values implementation
 *
 **/

#include "board_zobrist.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

/**
 * Initialized a set of random values for Zobrist hashing
 **/
void ZobristValues_initialize(ZobristValues* zobristValues, unsigned int seed)
{
  // Sets the seed
  srand(seed);

  for( int i=0; i<MAX_INTERSECTION_NUM; i++ ){
    zobristValues->black[i] = (unsigned long long)(rand() & 0xFF) | ((unsigned long long)(rand() & 0xFF) << 8) 
      |  ((unsigned long long)(rand() & 0xFF) << 16) |  ((unsigned long long)(rand() & 0xFF) << 24)
      | ((unsigned long long)(rand() & 0xFF) << 32) | ((unsigned long long)(rand() & 0xFF) << 40) 
      | ((unsigned long long)(rand() & 0xFF) << 48) |  ((unsigned long long)(rand() & 0xFF) << 56);
    zobristValues->white[i] = (unsigned long long)(rand() & 0xFF) | ((unsigned long long)(rand() & 0xFF) << 8) 
      |  ((unsigned long long)(rand() & 0xFF) << 16) |  ((unsigned long long)(rand() & 0xFF) << 24)
      | ((unsigned long long)(rand() & 0xFF) << 32) | ((unsigned long long)(rand() & 0xFF) << 40) 
      | ((unsigned long long)(rand() & 0xFF) << 48) |  ((unsigned long long)(rand() & 0xFF) << 56);
    zobristValues->ko[i] = (unsigned long long)(rand() & 0xFF) | ((unsigned long long)(rand() & 0xFF) << 8) 
      |  ((unsigned long long)(rand() & 0xFF) << 16) |  ((unsigned long long)(rand() & 0xFF) << 24)
      | ((unsigned long long)(rand() & 0xFF) << 32) | ((unsigned long long)(rand() & 0xFF) << 40) 
      | ((unsigned long long)(rand() & 0xFF) << 48) |  ((unsigned long long)(rand() & 0xFF) << 56);
  }

  zobristValues->turn = (unsigned long long)(rand() & 0xFF) | ((unsigned long long)(rand() & 0xFF) << 8) 
      |  ((unsigned long long)(rand() & 0xFF) << 16) |  ((unsigned long long)(rand() & 0xFF) << 24)
      | ((unsigned long long)(rand() & 0xFF) << 32) | ((unsigned long long)(rand() & 0xFF) << 40) 
      | ((unsigned long long)(rand() & 0xFF) << 48) |  ((unsigned long long)(rand() & 0xFF) << 56);
}
