/**
 * @file  board.h
 * @brief Provides types and functions to use for operating a Go board 
 *
 * Functions to generate move lists and to make moves on a Go board
 * are provided, along with utilities to operate on the board.
 * All operations are implemented to have best performance possible
 * to my current knowledge.
 *  
 **/
#ifndef BOARD_H
#define BOARD_H

#include "global_defs.h"
#include "stonegroup.h"
#include "hashTable.h"

#include <stdio.h>

/**
 * @brief Utility to retrieve a stone group by its pool index
 **/
#define STONE_GROUP(x) (&board->groups[x]) 

/**
 * @brief Representation of the pass move
 **/
#define PASS 400

/**
 * @brief The value that an intersection might have
 **/
typedef enum {
  BLACK,
  WHITE,
  EMPTY,
  BORDER
} Color;

/**
 * @brief Set of data used to iterate over a go board.
 * This data should be used as a constant, and does not need to 
 * be copied when the board is copied.
 *
 **/
typedef struct BoardIterator
{
  INTERSECTION intersections[MAX_INTERSECTION_NUM];
  int length;

} BoardIterator;

/**
 * @brief Represents a go board
 *
 * The board contains a map of all intersection values,
 * along with information about connected stones groups.
 **/
typedef struct Board
{

  /**
   * The size of the board's side
   **/
  unsigned char size;

  /**
   * Table of offsets used to compute neighbourgh
   * intersection index in all possible directions {N, E, S, W}
   **/
  signed char directionOffsets[4];

  /**
   * The map of intersections
   **/
  Color intersectionMap[MAX_INTERSECTION_NUM];

  /**
   * The pool of StoneGroup resources.
   * This resources are pointed by groupMap, and
   * represent all actual stone groups present on the board
   **/
  StoneGroup groups[MAX_STONEGROUPS];

  /**
   * Index of the first available (uninitialized) group in groups
   **/
  GRID firstAvailableGroup;

  /**
   * The map from coordinates to stoneGroups index.
   * For every intersection, the pool index of the group to which it 
   * belongs to (or GROUP_NULL for an empty) is mapped by this map.
   **/
  GRID groupMap[MAX_INTERSECTION_NUM];

  /**
   * The map from intersection coordinates to the groups (max 4)
   * to which that intersection is a liberty.
   **/
  GRID libertyMap[MAX_INTERSECTION_NUM][4];

  /**
   * List of empty squares
   **/
  short empties[MAX_INTERSECTION_NUM];

  /**
   * Number of empty squares
   **/
  short emptiesNum;

  /**
   * Current turn to play
   **/
  Color turn;

  /**
   * Captured black stones (white points)
   **/
  short whiteCaptures;

  /**
   * Captured white stones (black points)
   **/
  short blackCaptures;

  /**
   * The position of the current Ko (or -1 if not present)
   **/
  INTERSECTION koPosition;

  /**
   * Board's double-hash key
   **/
  HashKey hashKey;

} Board;

/**
 * @brief Utility to loop over all intersections of a board
 * 
 * All intersections that can be played (if board clear) and only those 
 * are browsed.
 *
 * Variable 'intersection' can be used in the loop to refer to the current
 * intersection being browsed.
 *
 **/
#define foreach_intersection(boardit) for(int i=0, intersection=boardit->intersections[0]; i<boardit->length; intersection=boardit->intersections[++i] )

/**
 * @brief Utility to loop over an intersection's neighbourgh.
 * 
 * Even in the case of a border or corner, the intersection index 
 * which correspond to the neighbourgh intersection is browsed, so
 * those cases should be checked inside the loop.
 *
 * Variable 'neigh' can be used in the loop to refer to the current
 * neighbourgh being browsed.
 *
 **/
#define foreach_neigh(board, intersection) for(int i=0, neigh=intersection+board->directionOffsets[0]; i<4; neigh=intersection+board->directionOffsets[++i] )

/**
 * @brief Utility to loop over the groups mapped as holders of an 
 * intersection as liberty.  That is, browse though all the groups who
 * have the specified intersection as a liberty.
 *
 * Variable 'grit' can be used in the loop to refer to the current group
 * being browsed.
 **/
#define foreach_libgroup(board, intersection) GRID grit=board->libertyMap[intersection][0]; for(int i=0; i<4 && (grit!=NULL_GROUP); grit=board->libertyMap[intersection][++i] )

#define foreach_empty(board) INTERSECTION empty=board->empties[0]; for(int i=0; i<board->emptiesNum; empty=board->empties[++i])

/**
 * @brief Initialize a new board of the specified size
 *
 * @param board The board to initialize
 * @param size The board's side size
 **/
void Board_initialize(Board* board, unsigned char size);

/**
 * @brief Entirely copy all board data from specified source to destination
 *
 * @param dst Destination board
 * @param src Source board
 **/
void Board_copy(Board* dst, Board* src);

/**
 * @brief Obtain the intersection coordinate index for the specified (x,y)
 *
 * @param board The board for which to compute the index
 * @param x Horizontal coordinate [0~board size-1]
 * @param y Vertical coordinate [0~board size-1]
 * @return A single integer value representing the pair of coordinates on the 
 * specified board
 **/
INTERSECTION Board_intersection(Board* board, int x, int y);

/**
 * @brief Retrieves the state of an intersection.
 *
 * @param board The board
 * @param intersection The intersection
 **/
Color Board_getColor(Board* board, INTERSECTION intersection);

/**
 * @brief Obtain the x cooardinate [0~board size-1] from an intersection index
 *
 * @param board The board
 * @param intersection The intersection index
 * @return the x coordinate [0~board size-1] that intersection represents
 **/
int Board_intersectionX(Board* board, INTERSECTION intersection);

/**
 * @brief Obtain the y cooardinate [0~board size-1] from an intersection index
 *
 * @param board The board
 * @param intersection The intersection index
 * @return the y coordinate [0~board size-1] that intersection represents
 **/
int Board_intersectionY(Board* board, INTERSECTION intersection);

/**
 * @brief Determines if playing on the specified intersection is legal
 *
 * @param board The board to play on 
 * param intersection The intersection to play on
 **/
int Board_isLegal(Board* board, INTERSECTION intersection);

/**
 * @brief Determines wether the next move for current turn's player 
 * must be a pass or not.
 * This function is SLOW, and not meant for performance
 *
 * @param board The board
 * @param iter A board iterator
 * @return 1 if there are no legal moves on the board, 0 otherwise
 **/
int Board_mustPass(Board* board, BoardIterator* iter);

/**
 * @brief Plays at the specified intersection of the board.
 * The specified intersection must be legal
 *
 * @param board The board to play on
 * @param intersection The intersection to play on
 **/
void Board_play(Board* board, INTERSECTION intersection);

/**
 * @brief Plays at the empty intersection that has the
 * specified ordinal index (emptyId) in the empties list.
 * This function is faster than Board_play because it does
 * not search the empty list.
 *
 * @param board The board to play on
 * @param emptyId The ordinal of the empty intersection in empties list
 * to play at
 **/
void Board_playEmpty(Board* board, int emptyId);

/**
 * @brief Calculates the hash-value of the child position of specified board
 * after the specified intersection would be played, without actually playing the move.
 * The board is guaranteed not to be modified.
 *
 * @param board The board to play on
 * @param intersection The intersection to virtually play on
 * @param outChildHash The hash value of the board after intersection would be played
 *
 **/
void Board_childHash(Board* board, INTERSECTION intersection, HashKey* outChildHash);

/**
 * @brief Plays a pass move on the specified board, which results
 * in swapping the turn's color.
 *
 * @param board The board to play on
 **/
void Board_pass(Board* board);

/**
 * @brief Returns the tromp-taylor score of a finished game, that is,
 * all dead stones must be removed from game and all dame must be filled.
 * 
 * @param board The board (game must be finished)
 * @param iterator A board iterator
 * @return The score difference from black's point of view.  If the game is not
 * tromp-taylor finished, behaviour is undefined.
 * 
 **/
int Board_trompTaylorScore(Board* board, BoardIterator* iterator);

/**
 * @brief Initializes a board iterator to be used over the current board.
 *
 * @param The board to iterate over
 * @param iterator The empty iterator to initialize
 **/
void Board_iterator(Board* board, BoardIterator* iterator);

/**
 * @brief Prints the board representation on the specified stream
 *
 * @param board The board to print
 * @param stream The stream to print to
 * @param withGroupInfo Prints debug groups informations as well
 * @param withLibertyMap Prints liberty map info as well
 **/
void Board_print(Board* board, FILE* stream, int withGroupInfo, int withLibertyMap);

/**
 * @brief Obtains a readable name of the specified intersection on the 
 * specified board
 *
 * @param board The board
 * @param intersection The intersection to name
 * @param nameBuffer An allocated buffer to which the readable name will be
 *        ouputted. The buffer must be at least 4 bytes long. Is the intersection
 *        is invalid, the string 'ERR' is outputted on nameBuffer
 **/
void Board_intersectionName(Board* board, INTERSECTION intersection, char* nameBuffer);

/**
 * @brief Calculate the intersection index for the specified intersection name
 * on the specified board.
 *
 * The name must be in the standard form 'Cdd' where C is a letter from A to the
 * last available letter on the specified board size, and dd is a 1 or 2 digit
 * number from 1 to board size. (ex. C13 on a 19x19 board).
 * Letters are contiguous without any irregularity (I and L are PRESENT).
 *
 * @param board The board
 * @param name The name in the standard form
 * @return The intersection index valid on the current board that corresponds to
 * the given intersection name. If the given name is invalid with respect to the
 * size of the given board, -1 is returned.
 **/
INTERSECTION Board_intersectionFromName(Board* board, char* name);

#endif
