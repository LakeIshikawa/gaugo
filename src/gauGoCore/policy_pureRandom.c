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

Color POLICY_pureRandom( UCTSearch* search, unsigned char* playedMoves )
{
  int passed = 0;
  for( int m=0; m<PLAYOUT_MOVES_MAX; m++ ){
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

      // Mark the move as played
      playedMoves[move] |= (!search->board->turn)+1;
    }
  }
}

