/**
 * @file board.c
 * @brief Board implementation
 *
 **/

#include "board.h"
#include "board_zobrist.h"
#include "crash.h"
#include "p3x3info.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// Static definitions
/** Zobrist static values */
ZobristValues zobrist1;
ZobristValues zobrist2;
int zobristInitialized = 0;

const int nodiags[4] = {1, 3, 4, 6};

// Board private methods declarations

/**
 * @brief Merge group at 'neigh' to newGroup updating maps, 
 * group stone and liberty counts, and removing the old group.
 *
 * @brief board The board
 * @brief newGroup The new group to merge to
 * @brief oldGroup The group to delete and merge
 **/
void Board_mergeGroups(Board* board, GRID newGroup, GRID oldGroup);

/**
 * @brief Removed all stones of the specified group from the board, 
 * including groupMap references and delete the group.
 *
 * @param board The board from which to kill the group
 * @param group The group to remove (pool index)
 **/
void Board_killGroup(Board* board, GRID group);

/**
 * @brief Place a stone on the board and create a new group of stones.
 * The new group is fully initialized with the stone and the liberties
 * around it.
 *
 * @param board The board
 * @param intersection The intersection to play on
 * @return The pool index of the new group initialized with the single 
 * stone and its liberties
 **/
GRID Board_placeStone(Board* board, INTERSECTION intersection);

/**
 * @brief Sets the ko position to the specified intersection, 
 * updating the hash key value accordingly.
 *
 * @param board The board
 * @param intersection The intersection to set to KO state
 **/
void Board_setKoPosition(Board* board, INTERSECTION intersection);

/**
 * @brief Unsets the ko position, updating the hash key value accordingly.
 *
 * @param board The board
 **/
void Board_unsetKoPosition(Board* board);

/**
 * @brief Sets the specified intersection state to the specified stone color,
 * updating the hash key value accordingly. This function makes no error-checking
 *
 * @param board The board
 * @param intersection The intersection to play on
 * @param color The color of the stone to place
 **/
void Board_setStone(Board* board, INTERSECTION intersection, Color color);

/**
 * @brief Sets the specified intersection state to the specified stone color,
 * updating the hash key value accordingly
 *
 * @param board The board
 * @param intersection The intersection on which to remove the stone
 **/
void Board_unsetStone(Board* board, INTERSECTION intersection);

/**
 * @brief If the given group is in atari, updates 3x3 hash of
 * the only move that is the liberty of the group with the atari
 * state of neighbor stones!
 * (thanks libego)
 **/
void Board_maybeAtari3x3(Board* board, GRID group);

/**
 * @brief Version for the group becoming not-atari
 **/
void Board_maybeAtariEnd3x3(Board* board, GRID group);

void Board_initialize(Board* board, unsigned char size)
{
  gauAssert(size <= MAX_BOARD_SIZE, board, NULL);

  // set the size and compute the direction offsets
  board->size = size;
  // Data used to compute neighbourgh intersection's indexes
  board->directionOffsets[0] = -board->size-2;
  board->directionOffsets[1] = -board->size-1;
  board->directionOffsets[2] = -board->size;
  board->directionOffsets[3] = -1;
  board->directionOffsets[4] = 1;
  board->directionOffsets[5] = board->size;
  board->directionOffsets[6] = board->size+1;
  board->directionOffsets[7] = board->size+2;

  // no initial ko
  board->koPosition = -1;
  // no captures
  board->whiteCaptures = 0;
  board->blackCaptures = 0;
  // black's turn at start
  board->turn = BLACK;
  // no last move
  board->lastMove = 0;

  // Clears hash (empty board is (1,1) 
  // to differentiate from unstored position(0,0))
  board->hashKey = 1;

  // Initialize zobrist values if needed
  if( !zobristInitialized ){
    ZobristValues_initialize(&zobrist1, 0x6bee2849 );
    ZobristValues_initialize(&zobrist2, 0xcb9561ca );
    zobristInitialized = 1;
  }

  // initialize the group map and groups
  for( int i=0; i<MAX_INTERSECTION_NUM; i++ ){
    board->groups[i].stonesNum = 0;
  }
  
  // initialize the intersections
  board->emptiesNum = 0;
  for( int i=0; i<MAX_INTERSECTION_NUM; i++ ){
    int x = i%(size+1);
    int y = i/(size+1);
    if( x == 0 || y == 0 || y > size ){
      board->intersectionMap[i] = BORDER;
    }
    else{
      board->intersectionMap[i] = EMPTY;

      // Add to empty list
      board->empties[board->emptiesNum++] = i;
      board->emptiesMap[i] = board->emptiesNum-1;
    }

    // Group map
    board->groupMap[i] = NULL_GROUP;
    board->nextStone[i] = 0;
  }

  // initialize patterns
  Board_initializePatterns(board);
}

void Board_initializePatterns(Board* board)
{
  for( int in=0; in<MAX_INTERSECTION_NUM; in++ ){
    // init to empty
    board->patterns3x3[in] = 0;

    int neigh;
    for( NEIGHBORS_DIAG(in) ){
      neigh = NEIGHI_DIAG(board, in);

      if( neigh>=0 && neigh<MAX_INTERSECTION_NUM ){
	Color nc = board->intersectionMap[neigh];
	if( nc != EMPTY ){
	  int cpatt = board->patterns3x3[in];
	  int mask = 3<<((7-i)*2);
	  if( nc == BORDER ){
	    board->patterns3x3[in] = (cpatt&~mask) | ((1<<((7-i)*2))&mask); 
	  }
	  else{
	    int bits = 2|nc;
	    board->patterns3x3[in] = cpatt&~mask | ((bits<<((7-i)*2))&mask);
	  }
	}
      }
    }

    // Atari bits
    for( NEIGHBORS(in) ){
      neigh = NEIGHI(board, in);
      if( board->intersectionMap[neigh] <= WHITE ){
	int atari = StoneGroup_isAtari(&board->groups[board->groupMap[neigh]]);
	board->patterns3x3[in] = board->patterns3x3[in] | (atari<<(16+(3-i)));
      }
    }
  }
}

void Board_copy(Board* dst, Board* src)
{
  *dst = *src;
  return;
}

INTERSECTION Board_intersection(Board* board, int x, int y)
{
  gauAssert( x>=0 && x < board->size && y >=0 && y < board->size, board, NULL );

  return (INTERSECTION)(((y+1) * (board->size+1)) + x+1);
}

int Board_intersectionX(Board* board, INTERSECTION intersection)
{
  return (intersection % (board->size+1))-1;
}

int Board_intersectionY(Board* board, INTERSECTION intersection)
{
  return (intersection / (board->size+1)) - 1;
}

Color Board_getColor(Board* board, INTERSECTION intersection)
{
  gauAssert( intersection >= 0 
	     && (intersection == PASS || intersection < MAX_INTERSECTION_NUM),
	     board, NULL);
  return board->intersectionMap[intersection];
}

int Board_isLegal(Board* board, INTERSECTION intersection)
{
#ifdef DEBUG
  // Check that intersection is a valid coordinate
  if( Board_intersectionX(board, intersection) < 0
      || Board_intersectionX(board, intersection) >= board->size
      || Board_intersectionY(board, intersection) < 0
      || Board_intersectionY(board, intersection) >= board->size){
    return 0;
  }
#endif  

  // If the intersection is not empty, the move is not legal
  // Ko move also not legal
  if( board->intersectionMap[intersection] != EMPTY
      || board->koPosition == intersection ) return 0;
  
  /* The move is legal if:
     a) It is touching an empty square
     b) It is killing any opponent group
     c) It is touching any friend group with more than 2 liberties
     
     If none of the above is true, the move is illegal
  */

  // a) touching an empty square
  if( Board_anyEmptyNeigh(board, intersection) ) return 1;

  // b&c) Remove a liberty from surrounding groups
  int neigh;
  for( NEIGHBORS(intersection ) ){
    neigh = NEIGHI(board, intersection);
    board->groups[board->groupMap[neigh]].libertiesNum--;
  }

  // Check for suicide
  int capture, not_suicide = 0;
  for( NEIGHBORS(intersection ) ){
    neigh = NEIGHI(board, intersection);
    capture = board->groups[board->groupMap[neigh]].libertiesNum == 0;
    // It is not suicide if you capture an opponent group
    // or it is not atari if any of your groups would not be killed by
    // your new move
    not_suicide |= capture != (board->intersectionMap[neigh] == board->turn); 
  }
  
  // Restore original liberties
  for( NEIGHBORS(intersection ) ){
    neigh = NEIGHI(board, intersection);
    board->groups[board->groupMap[neigh]].libertiesNum++;
  }
    
  return not_suicide;
}

int Board_isLegalNoEyeFilling(Board* board, INTERSECTION intersection)
{
  // Eye information in 3x3 info bits
  if( p3x3info[board->patterns3x3[intersection]] & (board->turn+1) ) 
    return 0;
  // Must be legal
  if( !Board_isLegal( board, intersection ) ) return 0;
  return 1;
}

int Board_anyEmptyNeigh(Board* board, INTERSECTION intersection)
{
  return p3x3info[board->patterns3x3[intersection]] >= 3;
}

int Board_mustPass(Board* board, BoardIterator* iter)
{
  INTERSECTION intersection;
  for( INTERSECTIONS(iter) ){
    intersection = INTERSECTIONI(iter);
    if( Board_isLegal(board, intersection) ) return 0;
  }

  return 1;
}

void Board_play(Board* board, INTERSECTION intersection)
{
  gauAssert(Board_isLegal(board, intersection), board, NULL);

  // Remember as last move
  board->lastMove = intersection;

  // Resets ko
  Board_unsetKoPosition(board);

  // Counters for captured stones number and place (if 1, for ko)
  short capturedStones = 0;
  INTERSECTION koPosition = -1;

  // Create a group for the new stone (which will be eventually merged)
  GRID unifiedGroup = Board_placeStone(board, intersection);

  // If new stone has 4 liberties, stop here
  if( board->groups[unifiedGroup].libertiesNum != 4 ){

    // Remove captured groups and merge friend chains
    for(NEIGHBORS(intersection)) {
      int neigh = NEIGHI(board, intersection);
      int neighgroup = board->groupMap[neigh];
      
      // Friend? merge
      if( board->intersectionMap[neigh] == board->turn 
	  && unifiedGroup != neighgroup ){
	if( board->groups[unifiedGroup].stonesNum
	    > board->groups[neighgroup].stonesNum ){
	  Board_mergeGroups(board, unifiedGroup, neighgroup);
	}
	else{
	  Board_mergeGroups(board, neighgroup, unifiedGroup);
	  unifiedGroup = neighgroup;
	}
      }
      // Opponent(captured)? kill
      else if( board->intersectionMap[neigh] == !board->turn ){
	if( StoneGroup_isCaptured(&board->groups[neighgroup]) ){
	  // Update captured stones and eventually save ko position
	  capturedStones += board->groups[neighgroup].stonesNum;
	  if( capturedStones == 1 ) koPosition = neigh;
	  
	  Board_killGroup( board, neighgroup );
	}
	// Might be in atari: update pattern
	else{
	  Board_maybeAtari3x3(board, neighgroup);
	}
      }
    }

    // Update captures
    if( capturedStones > 0 ){
      switch( board->turn ){
      case BLACK: board->blackCaptures += capturedStones; break;
      case WHITE: board->whiteCaptures += capturedStones; break;
      }
    }
    
    
    // If only 1 stone was capture, set the ko
    // only if placed stone's group has only 1 stone!
    if( capturedStones == 1 
	&& board->groups[unifiedGroup].stonesNum == 1){
      Board_setKoPosition(board, koPosition);
    }
  }

  // You may be in atari
  Board_maybeAtari3x3(board, unifiedGroup);
  
  // Swap turn
  board->turn = !board->turn;
}

void Board_maybeAtari3x3(Board* board, GRID group)
{
  // If not in atari, do nothing
  if( !StoneGroup_isAtari(&board->groups[group]) ) return;
  
  INTERSECTION atari = StoneGroup_atariLiberty(&board->groups[group]);  
  int atariBits = 
    ((board->groupMap[atari+board->directionOffsets[1]]==group )<< 19)
    | ((board->groupMap[atari+board->directionOffsets[3]]==group) << 18)
    | ((board->groupMap[atari+board->directionOffsets[4]]==group) << 17)
    | ((board->groupMap[atari+board->directionOffsets[6]]==group) << 16);
  
  board->patterns3x3[atari] |= atariBits;
}

void Board_maybeAtariEnd3x3(Board* board, GRID group)
{
  // If not in atari, do nothing
  if( !StoneGroup_isAtari(&board->groups[group]) ) return;
  // If the group is captured, do nothins
  if( StoneGroup_isCaptured(&board->groups[group]) ) return;

  INTERSECTION atari = StoneGroup_atariLiberty(&board->groups[group]);
  int atariBits = 
    ((board->groupMap[atari+board->directionOffsets[1]]==group )<< 19)
    | ((board->groupMap[atari+board->directionOffsets[3]]==group) << 18)
    | ((board->groupMap[atari+board->directionOffsets[4]]==group) << 17)
    | ((board->groupMap[atari+board->directionOffsets[6]]==group) << 16);
  
  board->patterns3x3[atari] &= ~atariBits;
}

HashKey Board_childHash(Board* board, INTERSECTION intersection)
{
  gauAssert(Board_isLegal(board, intersection), board, NULL);
  
  HashKey result = board->hashKey;

  // Hash-Play on the intersection
  switch( board->turn ){
  case BLACK:
    result ^= zobrist1.black[intersection];
    break;

  case WHITE:
    result ^= zobrist1.white[intersection];
    break;
  }

  for(NEIGHBORS(intersection)) {
    int neigh = NEIGHI(board, intersection);
    if( board->intersectionMap[neigh] != !board->turn ) {
      continue;
    }
    
    int neighgroup = board->groupMap[neigh];
    if( StoneGroup_isAtari(&board->groups[neighgroup]) ){

      // Hash kill stone
      INTERSECTION stone;
      for( STONES(board, neighgroup) ){
	stone = STONEI();
	
	// Kill stone
	switch( board->intersectionMap[stone] ){
	case BLACK:
	  result ^= zobrist1.black[stone];
	  break;
	  
	case WHITE:
	  result ^= zobrist1.white[stone];
	  break;
	}
      }
    }
  }

  return result;
}

void Board_pass(Board* board)
{
  board->turn = !board->turn;

  // Not ko anymore
  Board_unsetKoPosition(board);
}

int Board_trompTaylorScore(Board* board, BoardIterator* it)
{
  int points = 0;
  
  INTERSECTION intersection;
  for( INTERSECTIONS(it) ){
    intersection = INTERSECTIONI(it);
    
    switch( board->intersectionMap[intersection] ){
    case BLACK: points++; break;
    case WHITE: points--; break;
    case EMPTY: 
      if( board->intersectionMap[intersection-1] == BLACK 
	  || board->intersectionMap[intersection+1] == BLACK ) points++;
      else points--;
      break;
    }
  }

  return points;
}

void Board_iterator(Board* board, BoardIterator* iterator)
{
  iterator->length = 0;
  
  for( int i=0; i<MAX_INTERSECTION_NUM; i++ ){
    if( board->intersectionMap[i] != BORDER ){
      iterator->intersections[iterator->length++] = i;
    }
  }
}

void Board_mergeGroups(Board* board, GRID newGroup, GRID oldGroup)
{
  // Merge stone number and delete old group
  board->groups[newGroup].stonesNum += board->groups[oldGroup].stonesNum;
  board->groups[newGroup].libertiesNum += board->groups[oldGroup].libertiesNum;
  board->groups[newGroup].libSum += board->groups[oldGroup].libSum;
  board->groups[newGroup].libSumSq += board->groups[oldGroup].libSumSq;
  board->groups[oldGroup].stonesNum = 0;

  // Update all stone's map and count liberties
  INTERSECTION mergeHead = board->groups[oldGroup].groupHead;
  INTERSECTION stone = 0;
  for( STONES( board, oldGroup) ){
    stone = STONEI();
    // Update maps
    board->groupMap[stone] = newGroup;
  }

  // Insert merged group's head right after new head
  INTERSECTION head = board->groups[newGroup].groupHead;
  INTERSECTION temp = board->nextStone[head];
  board->nextStone[head] = mergeHead;
  board->nextStone[stone] = temp;
}

void Board_killGroup(Board* board, GRID group)
{  
  // Deletes the group
  board->groups[group].stonesNum = 0;
  
  INTERSECTION stone;
  // Remove all stones
  for( STONES(board, group) ){
    stone = STONEI();
    // Kill stone
    Board_unsetStone(board, stone);

    int neigh;
    for( NEIGHBORS(stone) ){
      neigh = NEIGHI(board, stone);
      // Liberty add
      if( board->intersectionMap[neigh] == board->turn ){
	Board_maybeAtariEnd3x3(board, board->groupMap[neigh]);
	StoneGroup_addLib(&board->groups[board->groupMap[neigh]], stone);
      }
    }
  }
}

GRID Board_placeStone(Board* board, INTERSECTION intersection)
{
  // Initialize an available group
  GRID newGroup = intersection;

  // Sets the new group informations
  board->groups[newGroup].libertiesNum = 0;
  board->groups[newGroup].libSum = 0;
  board->groups[newGroup].libSumSq = 0;
  board->groups[newGroup].stonesNum = 1;
  board->groups[newGroup].groupHead = intersection;

  // Put the stone on the board
  Board_setStone(board, intersection, board->turn);
  board->groupMap[intersection] = newGroup;
  board->nextStone[intersection] = 0;

  // Determines and adds the liberties
  int neigh;
  for( NEIGHBORS(intersection) ) {
    neigh = NEIGHI(board, intersection);
    
    // If liberty, add it to current new group
    if( board->intersectionMap[neigh] == EMPTY ){
      StoneGroup_addLib(&board->groups[newGroup], neigh);
    } else {
      // If stone, decrease that group's liberties!
      // This means that shared liberties are decreased twice
      // or 3 times!! All ok
      StoneGroup_subLib(&board->groups[board->groupMap[neigh]], intersection);
    }
  }

  return newGroup;
}

void Board_setKoPosition(Board* board, INTERSECTION intersection)
{
  board->koPosition = intersection;
}

void Board_unsetKoPosition(Board* board)
{
  if( board->koPosition != -1 ){
    board->koPosition = -1;
  }
}

void Board_setStone(Board* board, INTERSECTION intersection, Color color)
{
  board->intersectionMap[intersection] = color;
  
  switch( color ){
  case BLACK:
    board->hashKey ^= zobrist1.black[intersection];
    break;

  case WHITE:
    board->hashKey ^= zobrist1.white[intersection];
    break;
  }

  // Remove empty from list
  board->emptiesNum--;
  board->emptiesMap[board->empties[board->emptiesNum]] = board->emptiesMap[intersection];
  board->empties[board->emptiesMap[intersection]] = board->empties[board->emptiesNum];

  // Update patterns
  int neigh;
  for( NEIGHBORS_DIAG(intersection) ){
    neigh = NEIGHI_DIAG(board, intersection);
    int mask = 3<<(i*2);
    int bits = (2|board->turn)<<(i*2);
    board->patterns3x3[neigh] = 
      (board->patterns3x3[neigh]&~mask) | (bits & mask);
  }
}

void Board_unsetStone(Board* board, INTERSECTION intersection)
{
  switch( board->intersectionMap[intersection] ){
  case BLACK:
    board->hashKey ^= zobrist1.black[intersection];
    break;

  case WHITE:
    board->hashKey ^= zobrist1.white[intersection];
    break;
  }

  board->intersectionMap[intersection] = EMPTY;
  board->groupMap[intersection] = NULL_GROUP;

  // Add new empty intersection to list
  board->emptiesMap[intersection] = board->emptiesNum;
  board->empties[board->emptiesNum++] = intersection;

  // Patterns update
  int neigh;
  for( NEIGHBORS_DIAG(intersection) ){
    neigh = NEIGHI_DIAG(board, intersection);
    int mask = 3<<(i*2);
    board->patterns3x3[neigh] = board->patterns3x3[neigh]&~mask;
  }
}

void Board_print(Board* board, FILE* stream, int withGroupInfo)
{
  // Prints turn
  fprintf(stream, "%s to play, ko@%d\n", 
	  board->turn == BLACK ? "X" : "O", board->koPosition);

  // Prints hash
  fprintf(stream, "hash: %016llx\n", 
	  board->hashKey);

  // Prints row header
  fprintf(stream, "%-3s", "");

  for( int i=0; i<board->size; i++ ){
    char letter;
    if( 'A'+i >= 'I') letter = 'A'+i+1;
    else letter = 'A'+i;
    fprintf(stream, "%c ", letter);
  }

  fprintf(stream, "\n");

  // Prints every line
  for( int y=0; y<board->size; y++ ){
    fprintf(stream, "%-3d", board->size-y);
    // Prints every intersection
    for( int x=0; x<board->size; x++ ){
      int intersection = 
	board->intersectionMap[Board_intersection(board, x, y)];
      fprintf(stream, "%c ",
	      intersection == BLACK ? 'X' : 
	      intersection == EMPTY ? '-' : 
	      intersection == WHITE ? 'O' : 'E');
    }    
    fprintf(stream, "\n");
  }
  fprintf(stream, "\n");

  // Print group informations
  if( withGroupInfo ){
    fprintf(stream, "%-3s %-5s %-5s %s\n", 
	    "id", "libs", "nstn", "stones");

  
    for( int g=0; g<MAX_INTERSECTION_NUM; g++ ){
      StoneGroup* group = &board->groups[g];
      if( group->stonesNum > 0 ){
	fprintf(stream, "%-3d %-5d %-5d ", 
		g, group->libertiesNum, group->stonesNum);
    
	for( STONES( board, g ) ){
	  char name[5]; 
	  Board_intersectionName(board, STONEI(), name);
	  fprintf(stream, "%s ", name);
	}
	fprintf(stream, "\n");
      }
    }
  }

  // Empties
  fprintf(stream, "Empties: [");
  int empty;
  for( EMPTIES(board) ){
    empty = EMPTYI(board);
    char name[5];
    Board_intersectionName( board, empty, name );
    fprintf(stream, "%s ", name);
  }
  fprintf(stream, "]\n");
}

void Board_intersectionName(Board* board, INTERSECTION intersection, 
			    char* nameBuffer)
{
  if( intersection == PASS ){
    sprintf(nameBuffer, "pass");
    return;
  }

  int x = Board_intersectionX(board, intersection);
  int y = Board_intersectionY(board, intersection);
  
  // Error check
  if( x<0 || y<0 || x>=board->size || y>=board->size){
    sprintf(nameBuffer, "ERR");
  }
  
  else{
    char col = 'A'+x;
    // Sunningly, no 'I' column in GTP
    if( col >= 'I' ) col++;

    sprintf(nameBuffer, "%c%d", col, board->size-y);
  }
}

INTERSECTION Board_intersectionFromName(Board* board, char* name)
{
  char ax = name[0];
  if( ax >= 'a' ) ax -= ('a' - 'A');
  
  // Stunningly, no I column in gtp
  if( ax >= 'I' ) ax--;

  int x = ax - 'A';
  int y = board->size-atoi(name+1);

  // If name is not proper, returns an error value
  if( x<0 || y<0 || x>=board->size || y>=board->size){
    return -1;
  }

  // The name is regular, so returns the intersection index
  return Board_intersection(board, x, y);
}
