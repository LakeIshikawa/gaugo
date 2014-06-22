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
  return Board_isLegalNoEyeFilling( board, move );
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
      Board_play( board, intersection );
      return intersection;
    }
  }

  for(int i=0; i<randomPoint; i++){
    int intersection = board->empties[i];
    // Play it
    if( pureRandom_isPlayableMove( board,  intersection ) ){
      // play move
      Board_play( board, intersection );
      return intersection;
    }
  }
  
  // No moves available - pass
  Board_pass( board );
  return PASS;
}

Color POLICY_pureRandom( Board* board, BoardIterator* iter, 
			 float komi, unsigned char* playedMoves )
{
  int passed = 0;
  for( int m=0; m<PLAYOUT_MOVES_MAX; m++ ){
    INTERSECTION move = pureRandom_playRandom( board );

    if( move == PASS ){
      if( passed ){
	// Endgame!
	int score = (float)Board_trompTaylorScore( board, iter );
	return (score > komi) ? BLACK : WHITE;
      }
      passed = 1;
    } else {
      passed = 0;

      // Mark the move as played
      playedMoves[move] |= (!board->turn)+1;
    }
  }
}

