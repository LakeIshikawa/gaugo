/**
 * @file board.c
 * @brief Board implementation
 *
 **/

#include "board.h"
#include "board_zobrist.h"
#include "crash.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// Static definitions
/** Zobrist static values */
ZobristValues zobrist1;
ZobristValues zobrist2;
int zobristInitialized = 0;


// Board private methods declarations

/**
 * @brief Merges all groups neighbours of 'start' into 'newGroup'.
 * Stone and liberties will all be transfered to newGroup, and all
 * neighbour groups will be deleted
 *
 * @param newGroup The new group (1-stone brand new)
 * @aram start A stone of either groups to merge
 **/
void Board_mergeGroup(Board* board, GRID newGroup, INTERSECTION start);

/**
 * @brief Recalculate number of liberties of a string recursively.
 * Also, sets all stone's group map to 'newGroup'
 * 
 * @param board The board
 * @param newGroup Group map of merged group's stones will be set to this
 * @param start A stone of the string to calculate libs of
 * @param libmap A bitmap of size MAX_INTERSECTION_NUM to store 
 * the state of recursion
 **/
int Board_recalculateLiberties(Board* board, GRID newGroup,
			       INTERSECTION start, unsigned char* libmap);

/**
 * @brief Removed all stones of the specified group from the board, 
 * including groupMap references and delete the group.
 *
 * @param board The board from which to kill the group
 * @param group The group to remove (pool index)
 * @param start A stone of the group to be killed
 **/
void Board_killGroup(Board* board, GRID group, INTERSECTION start);

/**
 * @brief Kills the specified stone and recursively kills all
 * neighbourghs, adding new liberties to surrounding groups as needed
 *
 * @param board The board
 * @param stone The stone to kill
 **/
void Board_killStone(Board* board, INTERSECTION stone);

/**
 * @brief Place a stone on the board and create a new group of stones.
 * The new group is fully initialized with the stone and the liberties
 * around it.
 *
 * @param board The board
 * @param emptyId The ordinal of the empty intersection to be added to the new group
 * @return The pool index of the new group initialized with the single stone and its liberties
 **/
GRID Board_placeStone(Board* board, int emptyId);

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
 * @param emptyId The ordinal of the empty intersection on which to place the stone
 * @param color The color of the stone to place
 **/
void Board_setStone(Board* board, int emptyId, Color color);

/**
 * @brief Sets the specified intersection state to the specified stone color,
 * updating the hash key value accordingly
 *
 * @param board The board
 * @param intersection The intersection on which to remove the stone
 **/
void Board_unsetStone(Board* board, INTERSECTION intersection);

void Board_initialize(Board* board, unsigned char size)
{
  gauAssert(size <= MAX_BOARD_SIZE, board, NULL);

  // set the size and compute the direction offsets
  board->size = size;
  // Data used to compute neighbourgh intersection's indexes {N, E, S, W}
  board->directionOffsets[0] = -board->size-1;
  board->directionOffsets[1] = 1;
  board->directionOffsets[2] = board->size+1;
  board->directionOffsets[3] = -1;

  // no initial ko
  board->koPosition = -1;
  // no captures
  board->whiteCaptures = 0;
  board->blackCaptures = 0;
  // black's turn at start
  board->turn = BLACK;
  // no groups
  board->firstAvailableGroup = 0;

  // Clears hash (empty board is (1,1) 
  // to differentiate from unstored position(0,0))
  board->hashKey.key1 = 1;
  board->hashKey.key2 = 1;

  // Initialize zobrist values if needed
  if( !zobristInitialized ){
    ZobristValues_initialize(&zobrist1, 0x6bee2849 );
    ZobristValues_initialize(&zobrist2, 0xcb9561ca );
    zobristInitialized = 1;
  }

  // initialize the group map and groups
  for( int i=0; i<MAX_STONEGROUPS; i++ ){
    board->groups[i].stonesNum = 0;
  }
  
  // initialize the intersections
  board->emptiesNum = 0;
  for( int i=0; i<MAX_INTERSECTION_NUM; i++ ){
    int x = i%(size+1);
    int y = i/(size+1);
    if( x == size || y == 0 || y > size ){
      board->intersectionMap[i] = BORDER;
    }
    else{
      board->intersectionMap[i] = EMPTY;

      // Add to empty list
      board->empties[board->emptiesNum++] = i;
    }

    // Group map
    board->groupMap[i] = NULL_GROUP;
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

  return (INTERSECTION)(((y+1) * (board->size+1)) + x);
}

int Board_intersectionX(Board* board, INTERSECTION intersection)
{
  return intersection % (board->size+1);
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
  // Check that intersection is a valid coordinate
  if( Board_intersectionX(board, intersection) < 0
      || Board_intersectionX(board, intersection) >= board->size
      || Board_intersectionY(board, intersection) < 0
      || Board_intersectionY(board, intersection) >= board->size){
    return 0;
  }
  
  // If the intersection is not empty, the move is not legal
  if( board->intersectionMap[intersection] != EMPTY ) return 0;
  // Ko move not legal
  if( board->koPosition == intersection ) return 0;
  
  /* The move is legal if:
     a) It is touching an empty square
     b) It is killing any opponent group
     c) It is touching any friend group with more than 2 liberties
     
     If none of the above is true, the move is illegal
  */

  // Check all directions!
  foreach_neigh(board, intersection){
    if( board->intersectionMap[neigh] == EMPTY ) return 1;
    
    StoneGroup* neighbourgh = STONE_GROUP(board->groupMap[neigh]);
    if( board->intersectionMap[neigh] == board->turn 
	&& neighbourgh->libertiesNum >= 2 ) return 1;
    if( board->intersectionMap[neigh] == !board->turn 
	&& neighbourgh->libertiesNum == 1 ) return 1;
  }

  // None of a) b) c) holds, therefore the move should be suicide
  return 0;
}

int Board_isLegalNoEyeFilling(Board* board, INTERSECTION intersection)
{
  if( !Board_isLegal( board, intersection ) ) return 0;

  foreach_neigh( board, intersection ){
    Color nc = Board_getColor( board, neigh );
    if( nc == EMPTY || nc == !board->turn ) return 1;

    // If self stone in atari, its OK to fill eye
    if( STONE_GROUP( board->groupMap[neigh] )->libertiesNum == 1 ) return 1;
  }

  return 0;
}

int Board_mustPass(Board* board, BoardIterator* iter)
{
  foreach_intersection(iter){
    if( Board_isLegal(board, intersection) ) return 0;
  }

  return 1;
}

void Board_play(Board* board, INTERSECTION intersection)
{
  for(int i=0; i<board->emptiesNum; i++ ){
    if( board->empties[i] == intersection ){
      Board_playEmpty(board, i);
      return;
    }
  }

  gauAssert(0, board, NULL);
}

void Board_playEmpty(Board* board, int emptyId)
{
  INTERSECTION intersection = board->empties[emptyId];
  gauAssert(Board_isLegal(board, intersection), board, NULL);

  // Resets ko
  Board_unsetKoPosition(board);

  // Counters for captured stones number and palce (if 1, for ko)
  short capturedStones = 0;
  INTERSECTION koPosition = -1;

  // Create a group for the new stone (which will be eventually merged)
  int unifiedGroup = Board_placeStone(board, emptyId);
  
  // If new stone has 4 liberty, stop here
  if( board->groups[unifiedGroup].libertiesNum != 4 ){

    /**
     * Merge all surrounding groups with new group
     **/
    Board_mergeGroup(board, unifiedGroup, intersection);
    
    /** 
     * Removes the liberty corresponding to the played intersection, for all the
     * opponent groups which have a liberty on the played intersection.
     *
     * Also kills a group if it has no more liberties.
     **/
    GRID ar[4] = {NULL_GROUP, NULL_GROUP, NULL_GROUP, NULL_GROUP};
    int ari=0;
    foreach_neigh(board, intersection){
      // Skip non-opponent groups
      if( board->intersectionMap[neigh] != !board->turn ) continue;
      
      int neighgroup = board->groupMap[neigh];
      
      // Skip already decreased groups
      int skip = 0;
      for( int j=0; j<ari; j++ ){
	if( neighgroup == ar[j] ) {
	  skip = 1;
	  break;
	}
      }
      if( skip ) continue;
      
      ar[ari++] = neighgroup;
      board->groups[neighgroup].libertiesNum--;
      
      if( board->groups[neighgroup].libertiesNum == 0 ){
	// Update captured stones and eventually save ko position
	capturedStones += board->groups[neighgroup].stonesNum;
	if( capturedStones == 1 ) koPosition = neigh;
	
	Board_killGroup( board, neighgroup, neigh );
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

  // Swap turn
  board->turn = !board->turn;
  board->hashKey.key1 ^= zobrist1.turn;
  board->hashKey.key2 ^= zobrist2.turn;  
}

void Board_pass(Board* board)
{
  board->turn = !board->turn;
  board->hashKey.key1 ^= zobrist1.turn;
  board->hashKey.key2 ^= zobrist2.turn;
}

int Board_trompTaylorScore(Board* board, BoardIterator* it)
{
  int points = board->blackCaptures - board->whiteCaptures;
  
  foreach_intersection(it){
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

void Board_mergeGroup(Board* board, GRID newGroup, INTERSECTION start)
{  
  int anythingMerged = 0;

  foreach_neigh( board, start ){
    int neighgroup = board->groupMap[neigh];
      
    // if neighbourgh intersection is a friend stone, merge all!
    if( board->intersectionMap[neigh] == board->turn ){
      board->groups[newGroup].stonesNum += board->groups[neighgroup].stonesNum;
      board->groups[neighgroup].stonesNum = 0;
      anythingMerged = 1;
    }
  }

  // If nothing was merged, terminate
  if( !anythingMerged ) return;

  // Recalculate stones and liberties
  unsigned char libs[MAX_INTERSECTION_NUM];
  memset(libs, 0, sizeof(libs));
  
  board->groups[newGroup].libertiesNum = 
    Board_recalculateLiberties( board, newGroup, start, libs );
}

int Board_recalculateLiberties(Board* board, 
			       GRID newGroup,
			       INTERSECTION start, 
			       unsigned char* libmap) 
{
  // Dont recurse twice on the same stone
  if( libmap[start] & 2 ) return 0;
  libmap[start] |= 2;

  int libs = 0;
  foreach_neigh(board, start) {
    // Liberty
    if( board->intersectionMap[neigh] == EMPTY ){
      if( !(libmap[neigh]&1) ){
	// Flag the liberty so it won't be added twice
	libmap[neigh] |= 1;
	libs++;
      }
    }
    
    // Friend stone: recursion
    else if( board->intersectionMap[neigh] == board->turn ){
      libs += Board_recalculateLiberties(board, newGroup, neigh, libmap);
      // Also set groupmap to new group
      board->groupMap[neigh] = newGroup;
    }
  } 

  return libs;
}

void Board_killGroup(Board* board, GRID group, INTERSECTION start)
{
  // Deletes the group
  board->groups[group].stonesNum = 0;

  Board_killStone(board, start);
}

void Board_killStone(Board* board, INTERSECTION stone)
{
  // Do not kill twice
  if( board->intersectionMap[stone] == EMPTY ) return;
  // Kill stone
  Board_unsetStone(board, stone);
  
  // Give liberty to any surrounding group
  int aa[4] = {NULL_GROUP, NULL_GROUP, NULL_GROUP, NULL_GROUP};
  int aai = 0;
  
  foreach_neigh(board, stone) {
    // Liberty add
    if( board->intersectionMap[neigh] == board->turn ){

      // Skip already added groups
      int skip = 0;
      for( int j=0; j<aai; j++ ){
	if( board->groupMap[neigh] == aa[j] ) {
	  skip = 1;
	  break;
	}
      }
      if( skip ) continue;

      aa[aai++] = board->groupMap[neigh];
      board->groups[board->groupMap[neigh]].libertiesNum++;
    }

    // Another dead - recursion
    else if( board->intersectionMap[neigh] == !board->turn ) {
      Board_killStone(board, neigh);
    } 
  }
}

GRID Board_placeStone(Board* board, int emptyId)
{
  INTERSECTION intersection = board->empties[emptyId];

  // Initialize an available group
  GRID newGroup = board->firstAvailableGroup;

  // Adjust the firstAvailableGroup pointer
  do board->firstAvailableGroup = 
       (board->firstAvailableGroup+1) % MAX_STONEGROUPS; 
  while (board->groups[board->firstAvailableGroup].stonesNum != 0);

  // Sets the new group informations
  board->groups[newGroup].libertiesNum = 0;
  board->groups[newGroup].stonesNum = 1;

  // Put the stone on the board
  Board_setStone(board, emptyId, board->turn);
  board->groupMap[intersection] = newGroup;

  // Determines and adds the liberties
  foreach_neigh(board, intersection){
    if( board->intersectionMap[neigh] == EMPTY ){
      board->groups[newGroup].libertiesNum++;
    }
  }

  return newGroup;
}

void Board_setKoPosition(Board* board, INTERSECTION intersection)
{
  board->koPosition = intersection;
  board->hashKey.key1 ^= zobrist1.ko[intersection];
  board->hashKey.key2 ^= zobrist2.ko[intersection];
}

void Board_unsetKoPosition(Board* board)
{
  if( board->koPosition != -1 ){
    board->hashKey.key1 ^= zobrist1.ko[board->koPosition];
    board->hashKey.key2 ^= zobrist2.ko[board->koPosition];
    
    board->koPosition = -1;
  }
}

void Board_setStone(Board* board, int emptyId, Color color)
{
  INTERSECTION intersection = board->empties[emptyId];
  board->intersectionMap[intersection] = color;
  
  switch( color ){
  case BLACK:
    board->hashKey.key1 ^= zobrist1.black[intersection];
    board->hashKey.key2 ^= zobrist2.black[intersection];
    break;

  case WHITE:
    board->hashKey.key1 ^= zobrist1.white[intersection];
    board->hashKey.key2 ^= zobrist2.white[intersection];
    break;
  }

  // Remove empty from list
  board->empties[emptyId] = board->empties[--board->emptiesNum];
}

void Board_unsetStone(Board* board, INTERSECTION intersection)
{
  switch( board->intersectionMap[intersection] ){
  case BLACK:
    board->hashKey.key1 ^= zobrist1.black[intersection];
    board->hashKey.key2 ^= zobrist2.black[intersection];
    break;

  case WHITE:
    board->hashKey.key1 ^= zobrist1.white[intersection];
    board->hashKey.key2 ^= zobrist2.white[intersection];
    break;
  }

  board->intersectionMap[intersection] = EMPTY;
  board->groupMap[intersection] = NULL_GROUP;

  // Add new empty intersection to list
  board->empties[board->emptiesNum++] = intersection;
}

void Board_print(Board* board, FILE* stream, int withGroupInfo)
{
  // Prints turn
  fprintf(stream, "%s to play, ko@%d\n", 
	  board->turn == BLACK ? "X" : "O", board->koPosition);

  // Prints hash
  fprintf(stream, "hash: %016llx : %016llx\n", 
	  board->hashKey.key1, board->hashKey.key2);

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
    fprintf(stream, "%-3d", y+1);
    // Prints every intersection
    for( int x=0; x<board->size; x++ ){
      int intersection = board->intersectionMap[Board_intersection(board, x, y)];
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
    fprintf(stream, "%-3s %-5s %-5s\n", 
	    "id", "libs", "nstn");

  
    for( int g=0; g<MAX_STONEGROUPS; g++ ){
      StoneGroup* group = &board->groups[g];
      if( group->stonesNum > 0 ){
	fprintf(stream, "%-3d %-5d %-5d\n\n", 
		g, group->libertiesNum, group->stonesNum);
      }
    }
  }

  // Empties
  fprintf(stream, "Empties: [");
  foreach_empty(board){
    char name[4];
    Board_intersectionName( board, empty, name );
    fprintf(stream, "%s ", name);
  }
  fprintf(stream, "]\n");
}

void Board_intersectionName(Board* board, INTERSECTION intersection, 
			    char* nameBuffer)
{
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

    sprintf(nameBuffer, "%c%d", col, y+1);
  }
}

INTERSECTION Board_intersectionFromName(Board* board, char* name)
{
  char ax = name[0];
  if( ax >= 'a' ) ax -= ('a' - 'A');
  
  // Stunningly, no I column in gtp
  if( ax >= 'I' ) ax--;

  int x = ax - 'A';
  int y = atoi(name+1)-1;

  // If name is not proper, returns an error value
  if( x<0 || y<0 || x>=board->size || y>=board->size){
    return -1;
  }

  // The name is regular, so returns the intersection index
  return Board_intersection(board, x, y);
}
