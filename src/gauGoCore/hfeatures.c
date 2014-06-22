/**
 * @file features.c
 * @brief Features implementation
 *
 **/

#include "hfeatures.h"
#include <stdlib.h>

int simm[] = { 3, 2, 1, 0 };

int Features_capture(Board* board, INTERSECTION move)
{
  int neigh, neighsimm, stone, j;
  for( NEIGHBORS(move) ){
    neigh=NEIGHI(board, move);
    neighsimm=NEIGHI_SIMM(board, move);
    int neighgroup = board->groupMap[neigh];
    
    if( board->intersectionMap[neigh] == !board->turn ){   
      
      // Re-capture
      if( neigh == board->lastMove 
	  && board->previousWasCapture
	  && board->groups[neighgroup].libertiesNum == 1 ){
	return 2;
      }
      
      // Prevent connection
      if( neigh == board->lastMove
	  && board->groups[neighgroup].libertiesNum == 1
	  && board->intersectionMap[neighsimm] == !board->turn ){
	return 3;
      }

      // Contiguous to new-atari friend string
      if( board->groups[neighgroup].libertiesNum == 1 ){
	for( STONES(board, board->groupMap[neighgroup]) ){
	  for( NEIGHBORS2(move, j) ){
	    int neigh2 = NEIGHI2(board, move, j);
	    
	    for( int k=0; k<3; k++ ){
	      if( board->groupMap[neigh2] == board->newAtari[k] ){
		return 1;
	      }
	    }
	  }
	}
      }

    }
  }

  return 0;
}

int Features_extension(Board* board, INTERSECTION move)
{
  int neigh;
  for( NEIGHBORS(move) ){
    neigh=NEIGHI(board, move);
    int neighgroup = board->groupMap[neigh];
    
    if( board->intersectionMap[neigh] == !board->turn ){
      if( board->groups[neighgroup].libertiesNum == 1 ){
	for( int j=0; j<3; j++ ){
	  if( neighgroup == board->newAtari[j] ) 
	    return 1;
	}
      }
    }
  }

  return 0;
}

int Features_selfAtari(Board* board, INTERSECTION move)
{
  int neigh, newLibs=0, friendsLibs=0;
  for( NEIGHBORS(move) ){
    neigh=NEIGHI(board, move);
    if( board->intersectionMap[neigh] == EMPTY ) newLibs++;
    else if( board->intersectionMap[neigh] == board->turn ){   
      int neighgroup = board->groupMap[neigh];
      friendsLibs += board->groups[neighgroup].libertiesNum;
    }
  }

  return (friendsLibs-1) > newLibs;
}

int Features_atari(Board* board, INTERSECTION move)
{
  int neigh;
  for( NEIGHBORS(move) ){
    neigh=NEIGHI(board, move);
    if( board->intersectionMap[neigh] == !board->turn 
	&& board->groups[board->groupMap[neigh]].libertiesNum == 2){
      return 1;
    }
  }

  return 0;
}

int Features_distancePrevious(Board* board, INTERSECTION move)
{
  int dx = Board_intersectionX(board, move) 
    - Board_intersectionX(board, board->lastMove);
  int dy = Board_intersectionY(board, move) 
    - Board_intersectionY(board, board->lastMove);

  int dist = abs(dx)+abs(dy)+MAX(abs(dx), abs(dy));
  if( dist<4 ) return dist;
  return 0;
}
