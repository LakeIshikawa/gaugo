/**
 * @file board.c
 * @brief Board implementation
 *
 **/

#include "board.h"
#include "board_zobrist.h"
#include "emptiesList.h"
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
 * @brief Merge two groups, storing the result in dest and 
 * deleting the src group.
 *
 * Stones and liberties of one group will be completely copied to the other, 
 * and then will be set to deleted.  Also, all groupMap pointers of the board that 
 * pointed to the deleted group will be modified to point to the new merged group.
 *
 * Which group is currently mantained and which one is deleted is determined by
 * the stone size of the two groups (the smallest group will be deleted).
 *
 * @param dest The group to which to connect (pool index)
 * @param src The group to be assimiliated to dest (pool index)
 * @return The pool index of the merged group, which corresponds to wether dest or src
 **/
GRID Board_mergeGroup(Board* board, GRID dst, GRID src);

/**

 * @brief Add the specified intersection as a liberty to the specified group
 * The liberty is added only if not already a liberty of the group.
 *
 * @param board The board
 * @param group The group to which to add the liberty (pool index)
 * @param intersection The empty intersection to add as a liberty
 **/
void Board_addGroupLiberty(Board* board, GRID group, INTERSECTION intersection);

/**
 * @brief Removes the specified liberty intersection from the specified group.
 *
 * @param board The board
 * @param group The group from which to remove the liberty (pool index)
 * @param intersection The liberty to remove
 **/
void Board_removeGroupLiberty(Board* board, GRID group, INTERSECTION intersection);

/**
 * @brief Remove the specified group mapping from the liberty map of the specified
 * intersection.  The liberty does NOT get removed from the group's liberties set.
 *
 * @param board The board
 * @param group The group for which the mapping should be removed (pool index)
 * @param intersection The liberty intersection to remove the mapping from
 *
 **/
void Board_removeGroupLibertyMap(Board* board, GRID group, INTERSECTION intersection);

/**
 * @brief Add the specified intersection stone to the specified group.
 * This function does not deal with updating the liberties, for which case
 * the Board_mergeBoard function should be used.
 *
 * @param board The board
 * @param group The group to which to add the stone (pool index)
 * @param stone The intersection (stone) to add to the group
 **/
void Board_addGroupStone(Board* board, GRID group, INTERSECTION stone);

/**
 * @brief Removed all stones of the specified group from the board, 
 * including groupMap references and delete the group.
 *
 * @param board The board from which to kill the group
 * @param updateEmptiesList An optional empty list to update with played move
 * @param group The group to remove (pool index)
 **/
void Board_killGroup(Board* board, GRID group, EmptiesList* updateEmptiesList);

/**
 * @brief Place a stone on the board and create a new group of stones.
 * The new group is fully initialized with the stone and the liberties
 * around it.
 *
 * @param board The board
 * @param intersection The intersection to be added to the new group
 * @param updateEmptiesList An optional empty list to update with played move
 * @return The pool index of the new group initialized with the single stone and its liberties
 **/
GRID Board_placeStone(Board* board, INTERSECTION intersection, EmptiesList* updateEmptiesList);

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
 * @param intersection The intersection on which to place the stone
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

  // Clears hash (empty board is (1,1) to differentiate from unstored position(0,0))
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
  for( int i=0; i<MAX_INTERSECTION_NUM; i++ ){
    int x = i%(size+1);
    int y = i/(size+1);
    if( x == size || y == 0 || y > size ){
      board->intersectionMap[i] = BORDER;
    }
    else{
      board->intersectionMap[i] = EMPTY;
    }

    // Liberty map
    for( int j=0; j<4; j++ ){
      board->libertyMap[i][j] = NULL_GROUP;
    }

    // Group map
    board->groupMap[i] = NULL_GROUP;
  }
}

void Board_copy(Board* dst, Board* src)
{
  *dst = *src;
  return;

  // Copy the size
  dst->size = src->size;
  // Copy direction offsets
  memcpy(dst->directionOffsets, src->directionOffsets, sizeof(src->directionOffsets));

  // Copy ko
  dst->koPosition = src->koPosition;
  // Copy captures
  dst->whiteCaptures = src->whiteCaptures;
  dst->blackCaptures = src->blackCaptures;
  // Copy turn
  dst->turn = src->turn;
  // Copy next group idx
  dst->firstAvailableGroup = src->firstAvailableGroup;

  // Copy hash
  memcpy(&dst->hashKey, &src->hashKey, sizeof( HashKey ));

  // Copy maps
  memcpy(dst->intersectionMap, src->intersectionMap, sizeof( src->intersectionMap ));
  memcpy(dst->groupMap, src->groupMap, sizeof( src->groupMap ));
  memcpy(dst->libertyMap, src->libertyMap, sizeof( src->libertyMap ));

  // Copy groups
  int i;
  for( i=0; i<src->firstAvailableGroup; i++ ){
    dst->groups[i].stonesNum = src->groups[i].stonesNum;
    // Stones
    memcpy( dst->groups[i].intersections, src->groups[i].intersections, 
	    sizeof(INTERSECTION)*src->groups[i].stonesNum );
    // Libs
    memcpy( dst->groups[i].liberties, src->groups[i].liberties, 
	    sizeof(INTERSECTION)*src->groups[i].libertiesNum );
  }

  // Set remaining groups to null
  for( ; i<MAX_STONEGROUPS; i++ ){
    dst->groups[i].stonesNum = 0;
  }  
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
  gauAssert( intersection >= 0 && (intersection == PASS || intersection < MAX_INTERSECTION_NUM),
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
    if( board->intersectionMap[neigh] == board->turn && neighbourgh->libertiesNum >= 2 ) return 1;
    if( board->intersectionMap[neigh] == !board->turn && neighbourgh->libertiesNum == 1 ) return 1;
  }

  // None of a) b) c) holds, therefore the move should be suicide
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
  Board_playUpdatingEmpties(board, intersection, NULL);
}

void Board_playUpdatingEmpties(Board* board, INTERSECTION intersection, EmptiesList* updateEmptiesList)
{
  gauAssert(Board_isLegal(board, intersection), board, NULL);

  // Resets ko
  Board_unsetKoPosition(board);

  // Counters for captured stones number and palce (if 1, for ko)
  short capturedStones = 0;
  INTERSECTION koPosition = -1;

  // Create a group for the new stone (which will be eventually merged)
  int unifiedGroup = Board_placeStone(board, intersection, updateEmptiesList);
    
  /** 
   * First browse neighbourghs of played stone to connect all 
   * surrounding friend groups into a single unified group.
   **/
  foreach_neigh(board, intersection){
    int neighgroup = board->groupMap[neigh];
      
    // if neighbourgh intersection is a friend stone, merge the two groups!
    if( board->intersectionMap[neigh] == board->turn && neighgroup != unifiedGroup){
      unifiedGroup = Board_mergeGroup(board, unifiedGroup, neighgroup);
    }
  }

  /** 
   * Removes the liberty corresponding to the played intersection, for all the
   * groups which have a liberty mapping to the played intersection.
   *
   * Also kills a group if it has no more liberties.
   **/
  foreach_libgroup(board, intersection){
    StoneGroup* stGroup = STONE_GROUP(grit);
    Board_removeGroupLiberty(board, grit, intersection);

    if( stGroup->libertiesNum == 0 ){
      // Update captured stones and eventually save ko position
      capturedStones += stGroup->stonesNum;
      if( capturedStones == 1 ) koPosition = stGroup->intersections[0];

      Board_killGroup( board, grit, updateEmptiesList );
    }
  }

  // Clear the liberty mapping for all groups that had the liberty
  for( int i=0; i<4 && board->libertyMap[intersection][i] != NULL_GROUP; i++ ){
    board->libertyMap[intersection][i] = NULL_GROUP;
  }
  
  // Update captures
  if( capturedStones > 0 ){
    switch( board->turn ){
    case BLACK: board->blackCaptures += capturedStones; break;
    case WHITE: board->whiteCaptures += capturedStones; break;
    }
  }

  // If only 1 stone was capture, set the ko
  if( capturedStones == 1 ){
    Board_setKoPosition(board, koPosition);
  }

  // Swap turn
  board->turn = !board->turn;
  board->hashKey.key1 ^= zobrist1.turn;
  board->hashKey.key2 ^= zobrist2.turn;  
}

void Board_childHash(Board* board, INTERSECTION intersection, HashKey* outChildHash)
{
  gauAssert(Board_isLegal(board, intersection), board, NULL);
  
  // Copy current hash value
  *outChildHash = board->hashKey;

  // If go is going on, clear it
  if( board->koPosition != -1 ){
    outChildHash->key1 ^= zobrist1.ko[board->koPosition];
    outChildHash->key2 ^= zobrist2.ko[board->koPosition];
  }

  // Hash-Play on the intersection
  switch( board->turn ){
  case BLACK:
    outChildHash->key1 ^= zobrist1.black[intersection];
    outChildHash->key2 ^= zobrist2.black[intersection];
    break;

  case WHITE:
    outChildHash->key1 ^= zobrist1.white[intersection];
    outChildHash->key2 ^= zobrist2.white[intersection];
    break;
  }

  // Look for kills
  INTERSECTION koPosition = -1;
  int capturedStones = 0;

  foreach_libgroup(board, intersection){
    StoneGroup* stGroup = STONE_GROUP(grit);

    if( board->intersectionMap[stGroup->intersections[0]] == !(board->turn) 
	&& stGroup->libertiesNum == 1 ){
      // Update captured stones and eventually save ko position
      capturedStones += stGroup->stonesNum;
      if( stGroup->stonesNum == 1 ) koPosition = stGroup->intersections[0];

      
      // Hash-kill the stones
      foreach_stone(stGroup){
	switch( board->intersectionMap[stone] ){
	case BLACK:
	  outChildHash->key1 ^= zobrist1.black[stone];
	  outChildHash->key2 ^= zobrist2.black[stone];
	  break;
	  
	case WHITE:
	  outChildHash->key1 ^= zobrist1.white[stone];
	  outChildHash->key2 ^= zobrist2.white[stone];
	  break;
	}
      }
    }
  }

  // Hash-set the ko if necessary
  if( capturedStones == 1 ){
    outChildHash->key1 ^= zobrist1.ko[koPosition];
    outChildHash->key2 ^= zobrist2.ko[koPosition];
  }

  // Turn
  outChildHash->key1 ^= zobrist1.turn;
  outChildHash->key2 ^= zobrist2.turn;
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

GRID Board_mergeGroup(Board* board, GRID dest, GRID src)
{
  StoneGroup* destPt = STONE_GROUP(dest);
  StoneGroup* srcPt = STONE_GROUP(src);

  // If dest is smaller than src, swap the pointers
  if( destPt->stonesNum < srcPt->stonesNum){
    StoneGroup* temp = destPt;
    destPt = srcPt;
    srcPt = temp;

    int tmp = dest;
    dest = src;
    src = tmp;
  }

  // All all unshared liberties of src to dest, and updates the liberty map
  foreach_liberty(srcPt){
    Board_removeGroupLibertyMap(board, src, liberty);
    Board_addGroupLiberty(board, dest, liberty);    
  }
  
  // Add all src stones to dest and set groupMap
  foreach_stone(srcPt){
    Board_addGroupStone(board, dest, stone);
  }

  // Delete src
  srcPt->stonesNum = 0;

  // return the merged group
  return dest;
}

void Board_addGroupStone(Board* board, GRID group, INTERSECTION stone){
  StoneGroup* stGroup = STONE_GROUP(group);
  stGroup->intersections[stGroup->stonesNum++] = stone;
  board->groupMap[stone] = group;
}

void Board_addGroupLiberty(Board* board, GRID group, INTERSECTION intersection)
{
  StoneGroup* stGroup = STONE_GROUP(group);

  // First check if liberty is already liberty of the group
  int nmaps=0;
  foreach_libgroup(board, intersection){
    if( grit == group ) return;
    nmaps=i+1;
  }

  // Liberty is a new liberty, so should be added
  stGroup->liberties[stGroup->libertiesNum++] = intersection;

  // Also add a new mapping for this group's lib
  gauAssert(nmaps < 4, board, NULL);
  board->libertyMap[intersection][nmaps] = group;
}

void Board_removeGroupLiberty(Board* board, GRID group, INTERSECTION intersection)
{
  StoneGroup* stGroup = STONE_GROUP(group);

  // Search and remove the liberty from the group's liberty set
  foreach_liberty(stGroup){
    if( liberty == intersection ){
      // Shift everything to the left
      for( int j=i+1; j<stGroup->libertiesNum; j++ ){
	stGroup->liberties[j-1] = stGroup->liberties[j];
      }

      break;
    }
  }

  // Remove one liberty
  stGroup->libertiesNum--;
}

void Board_removeGroupLibertyMap(Board* board, GRID group, INTERSECTION intersection)
{
  for( int i=0; i<4 && board->libertyMap[intersection][i]!=NULL_GROUP; i++ ){
    if( board->libertyMap[intersection][i] == group ){
      // Shift everything to the left
      for( int j=i+1; j<4; j++ ){
	board->libertyMap[intersection][j-1] = board->libertyMap[intersection][j];
      }
      
      // Clear the last (it must become empty)
      board->libertyMap[intersection][3] = NULL_GROUP;
      return;
    }
  }

  // The group does not appear in the specified liberty map
  gauAssert(0, board, NULL);
}

void Board_killGroup(Board* board, GRID group, EmptiesList* updateEmptiesList)
{
  StoneGroup* stGroup = STONE_GROUP(group);
  
  // Delete all groupMap references and sets the board to empties
  foreach_stone(stGroup){
    board->groupMap[stone] = NULL_GROUP;
    Board_unsetStone(board, stone);
  }

  foreach_stone(stGroup){

    // The removed stone becomes a new potential liberty,
    // so search in the neighbourhood for groups and eventually add
    // the new liberty.
    foreach_neigh(board, stone){
      if( board->groupMap[neigh] != NULL_GROUP ){
	Board_addGroupLiberty(board, board->groupMap[neigh], stone);
      }
    }

    // Add new empty intersection to list if needed
    if( updateEmptiesList ){
      EmptiesList_add( updateEmptiesList, stone );
    }
  }

  // Deletes the group
  stGroup->stonesNum = 0;
  
}

GRID Board_placeStone(Board* board, INTERSECTION intersection, EmptiesList* updateEmptiesList)
{
  // Initialize an available group
  GRID newGroup = board->firstAvailableGroup;
  StoneGroup* newGroupPt = STONE_GROUP(newGroup);

  // Adjust the firstAvailableGroup pointer
  do board->firstAvailableGroup = (board->firstAvailableGroup+1) % MAX_STONEGROUPS; 
  while (board->groups[board->firstAvailableGroup].stonesNum != 0);

  // Sets the new group informations
  newGroupPt->intersections[newGroupPt->stonesNum++] = intersection;
  newGroupPt->libertiesNum = 0;
  
  // Put the stone on the board
  Board_setStone(board, intersection, board->turn);
  board->groupMap[intersection] = newGroup;

  // Determines and adds the liberties
  foreach_neigh(board, intersection){
    if( board->intersectionMap[neigh] == EMPTY ){
      Board_addGroupLiberty(board, newGroup, neigh);
    }
  }

  // Remove empty from list
  if( updateEmptiesList ){
    EmptiesList_delete( updateEmptiesList, intersection );
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

void Board_setStone(Board* board, INTERSECTION intersection, Color color)
{
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
}

void Board_print(Board* board, FILE* stream, int withGroupInfo, int withLibertyMap)
{
  // Prints turn
  fprintf(stream, "%s to play, ko@%d\n", board->turn == BLACK ? "X" : "O", board->koPosition);

  // Prints hash
  fprintf(stream, "hash: %016llx : %016llx\n", board->hashKey.key1, board->hashKey.key2);

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

  // Print group informations
  if( withGroupInfo ){
    fprintf(stream, "%-3s %-5s %-5s %-40s %-40s\n", "id", "libs", "nstn", "stones", "libs");

  
    for( int g=0; g<MAX_STONEGROUPS; g++ ){
      StoneGroup* group = &board->groups[g];
      if( group->stonesNum > 0 ){
	fprintf(stream, "%-3d %-5d %-5d ", g, group->libertiesNum, group->stonesNum);

	// Print stone list
	char stoneStr[41];
	stoneStr[0] = '\0';

	foreach_stone(group){
	  char intStr[3];
	  Board_intersectionName(board, stone, intStr);

	  strcat(stoneStr, intStr);
	  if( i!=group->stonesNum-1 ) strcat(stoneStr, ",");
	}
	fprintf(stream, "%-40s ", stoneStr);

	// Print liberties list
	char libStr[41];
	libStr[0] = '\0';

	foreach_liberty(group){
	  char intStr[3];
	  Board_intersectionName(board, liberty, intStr);

	  strcat(libStr, intStr);
	  if( i!=group->libertiesNum-1 ) strcat(libStr, ",");
	}
	fprintf(stream, "%-40s\n", libStr);

      }
    }
  }

  // Liberty map info
  if( withLibertyMap ){
    for( int y=0; y<board->size; y++ ){
      for( int x=0; x<board->size; x++ ){
	int intersection = Board_intersection(board, x, y);
	char libmapStr[13];
	memset(libmapStr, 0, sizeof(libmapStr));
	
	foreach_libgroup(board, intersection){
	  sprintf(libmapStr + strlen(libmapStr), "%d ", grit);
	}
	
	if( strlen(libmapStr) != 0 ){
	  char name[4];
	  Board_intersectionName(board, intersection, name);
	  fprintf(stream, "%s:[%s] ", name, libmapStr);
	}
      }
      
      fprintf(stream, "\n");
    }
  }

}

void Board_intersectionName(Board* board, INTERSECTION intersection, char* nameBuffer)
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
