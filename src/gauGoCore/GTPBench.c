/**
 * @file  GTPBench.c
 * @brief Implementation of some benchmark GTP commands
 **/

#include <assert.h>
#include <stdlib.h>
#include "GTPBench.h"
#include "board.h"
#include "uctTree.h"
#include "timer.h"
#include "policies.h"

#define BENCH_POS 100000.0f

void GTPBench_playoutBench( GauGoEngine* engine, int argc, char** argv )
{
  Timer timer;

  Board boardCopy;
  Board_copy( &boardCopy, engine->board );  
  BoardIterator it;
  Board_iterator(&boardCopy, &it);

  // Do 30k po
  int blackWins = 0;
  unsigned char playedMoves[MAX_INTERSECTION_NUM];
  
  Timer_start( &timer );
  for( int po=0; po<BENCH_POS; po++ ){
    Board_copy( &boardCopy, engine->board );
    if( POLICY_pureRandom( &boardCopy, &it, 6.5f, playedMoves ) == BLACK ){
      blackWins++;
    }
  }

  int pps = BENCH_POS*1000 / Timer_getElapsedTime(&timer);
  printf("= %dpps\nwr=%f \n\n", pps, (blackWins/BENCH_POS));
  fflush(stdout);
}
