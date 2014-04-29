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

/**
 * @brief Play a random move uniformely over empty squares.
 **/
INTERSECTION pureRandom_playRandom(Board* board)
{
  // Linearly tries all moves from random point on
  int randomPoint = rand() % board->emptiesNum;
  
  for(int i=randomPoint; i<board->emptiesNum; i++){
    int intersection = board->empties[i];
    // Play it
    if( pureRandom_isPlayableMove( board,  intersection ) ){
      // play move
      Board_playEmpty( board, i );
      return intersection;
    }
  }

  for(int i=0; i<randomPoint; i++){
    int intersection = board->empties[i];
    // Play it
    if( pureRandom_isPlayableMove( board,  intersection ) ){
      // play move
      Board_playEmpty( board, i );
      return intersection;
    }
  }
  
  // No moves available - pass
  Board_pass( board );
  return PASS;
}

Color POLICY_pureRandom( UCTSearch* search )
{
  int passed = 0;
  while( 1 ){
    INTERSECTION move = pureRandom_playRandom( search->board );
    if( move == PASS ){
      if( passed ){
	// Endgame!
	int score = (float)Board_trompTaylorScore( search->board, search->iter );
	return (score > search->options->komi) ? BLACK : WHITE;
      }
      passed = 1;
    } else {
      passed = 0;
    }
  }
}

