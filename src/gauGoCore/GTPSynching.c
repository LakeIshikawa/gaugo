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

void GTPSynching_ucttree( GauGoEngine* engine, int argc, char** argv )
{
  printf("= ");
  
  UCTNode* pos = GauGoEngine_getTreePos( engine );
  if( pos ){
    foreach_child( pos ){
      char intName[4];
      Board_intersectionName( engine->board, child->move, intName );
      printf( "%s/%d/%d/%d/%d/%f,", 
	      intName, 
	      child->winsBlack, 
	      child->played,
	      child->AMAFwinsBlack,
	      child->AMAFplayed,
	      UCTNode_evaluateUCT( child, pos, !engine->board->turn, 0.44f )
	      );
    }
  }

  printf("\n\n");
  fflush(stdout);
}
