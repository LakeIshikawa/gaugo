/**
 * @file  policy_pureRandom.c
 * @brief Pure random playout policy implementation
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include "policies.h"

/**
 * @brief Determines if the specified move is legal and 
 * not self-eye filling.
 **/
int pureRandom_isPlayableMove( Board* board, INTERSECTION move )
{
  if( !Board_isLegal( board, move ) ) return 0;

  foreach_neigh( board, move ){
    Color nc = Board_getColor( board, neigh );
    if( nc == EMPTY || nc == !board->turn ) return 1;

    // If self stone in atari, its OK to fill eye
    if( STONE_GROUP( board->groupMap[neigh] )->libertiesNum == 1 ) return 1;
  }

  return 0;
}

Color POLICY_pureRandom( UCTSearch* search )
{
  // Move buffer
  INTERSECTION moveBuffer[MAX_INTERSECTION_NUM];
  int numMoves = 0;
  int passed = 0;

  // Number of empties at which to change policy
  int emptiesHorizon = (search->board->size*search->board->size) * 0.75f;
  int randomTries = 0;
  while( 1 ){
    // If many empties on the board still, first try 4 times randomly
    if( (randomTries<4) && search->emptiesList->length > emptiesHorizon ){
      for(; randomTries<4; randomTries++ ) {
	int randomMove = rand() % search->emptiesList->length;
	EmptyNode* it = search->emptiesList->firstEmpty;
	
	for( int i=0; i<randomMove; i++ ){
	  it = it->next;
	}
	
	if( pureRandom_isPlayableMove( search->board, it->intersection ) ){
	  // play move
	  Board_playUpdatingEmpties( search->board, it->intersection, search->emptiesList );
	  break;
	}
      }
    }

    
    // Generates all playable moves and randomly chooses one
    else{
      numMoves = 0;
      foreach_empty( search->emptiesList ){
	if( pureRandom_isPlayableMove( search->board, emptyNode->intersection ) ){
	  moveBuffer[numMoves++] = emptyNode->intersection;
	}
      }
      
      if( numMoves == 0 ){
	if( passed ){
	  // Endgame!
	  int score = (float)Board_trompTaylorScore( search->board, search->iter );
	  return (score > search->options->komi) ? BLACK : WHITE;
	}
	passed = 1;
	Board_pass( search->board );
      }

      else{
	// Randomly chooses and play move
	int randomMove = rand() % numMoves;
	// play move
	Board_playUpdatingEmpties( search->board, moveBuffer[randomMove], search->emptiesList );
	passed = 0;
      }    
    }
  }
}

