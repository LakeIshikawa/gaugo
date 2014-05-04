/**
 * @file  GTPSynching.c
 * @brief Implementation of some GTP synching commands
 **/

#include <assert.h>
#include "GTPSynching.h"
#include "board.h"

void GTPSynching_board( GauGoEngine* engine, int argc, char** argv )
{
  printf("= ");

  printf("BS=%d\n", engine->board->size);
  printf("TN=%d\n", engine->board->turn);
  printf("WC=%d\n", engine->board->whiteCaptures);
  printf("BC=%d\n", engine->board->blackCaptures);
  printf("KO=%d\n", engine->board->koPosition);
  
  // Board
  printf("BD=");

  BoardIterator it;
  Board_iterator(engine->board, &it);
  
  BoardIterator* itp = &it;
  foreach_intersection(itp){
    switch( Board_getColor( engine->board, intersection ) ){
    case BLACK: printf("X"); break;
    case WHITE: printf("O"); break;
    case EMPTY: printf("-"); break;
    case BORDER: printf("*"); break;
    }

    if( engine->board->intersectionMap[intersection+1] == BORDER ){
      printf("|");
    }
  }

  printf("\n\n");
  fflush(stdout);
}
