/**
 * @file  stoppers.c
 * @brief Some standard uct search stoppers implementations
 *
 **/

#include "stoppers.h"

int STOPPER_5ksim( UCTSearch* search, int simulations )
{
  // TEST
  //char buf[64];
  //Board_print( search->board, stdout, 0, 0 );
  //fgets( buf, 64, stdin );

  if( simulations % 3000 == 0 ){
    //printf("Hash: %d ent(%f) / %d coll \n", search->hashTable->entriesNum, 
    //	   (float)search->hashTable->entriesNum/search->hashTable->mask,
    //	   search->hashTable->firstKeyCollisions );
    //printf("End of sim: %d\n", simulations);
    
    // Prints pv
    UCTSearch_printPv( search );
    

    fflush(stdout);
  }
  return simulations >= 15000;
}

