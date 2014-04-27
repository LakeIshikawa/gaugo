/**
 * @file  emptiesList.h
 * @brief Provides types and functions to use for operating a list of empty intersections
 * on a go board
 *
 * 
 * The list is implemented by a linked list backed by a map, which allows fast
 * traversing and fast access/deletion of entries.
 **/
#ifndef EMPTIESLIST_H
#define EMPTIESLIST_H

#include "global_defs.h"
#include "board.h"

#include <stdio.h>

/**
 * @brief Represents an empty of the list
 **/
typedef struct EmptyNode
{
  INTERSECTION intersection;

  struct EmptyNode* previous;
  struct EmptyNode* next;
} EmptyNode;

/**
 * @brief Represents a list of (empty) intersections on a go board
 **/
typedef struct EmptiesList
{
  /** Empty node pool map */
  EmptyNode pool[MAX_INTERSECTION_NUM];

  /** The list */
  EmptyNode* firstEmpty;
  EmptyNode* lastEmpty;

  /** List actual size */
  int length;

} EmptiesList;

/**
 * @brief Utility to browse an empties list
 *
 * Use implicit variable 'emptyNode' to access current element.
 **/
#define foreach_empty( list ) for( EmptyNode* emptyNode = list->firstEmpty; emptyNode; emptyNode=emptyNode->next )

/**
 * @brief Initialized the specified move list with al legal moves 
 * from the specified board position.
 *
 * @param list The move list to be initialized
 * @param board The board position from which to generate legal move list
 **/
void EmptiesList_initialize( EmptiesList* list, BoardIterator* boardIter, Board* board );

/**
 * @brief Adds the specified intersection to the specified list
 *
 * @brief Adds the specified intersection as an empty to the list
 *
 * @param list The list
 * @param intersection The intersection to add
 **/
void EmptiesList_add( EmptiesList* list, INTERSECTION intersection );

/**
 * @brief Remove the empty corresponding to the specified intersection 
 * from the list.
 *
 * @param list The list
 * @param intersection The intersection to be removed
 *
 **/
void EmptiesList_delete( EmptiesList* list, INTERSECTION intersection );

/**
 * @brief Prints the list to stdout for debugging purposes.
 *
 * @param list The empties list
 **/
void EmptiesList_print( EmptiesList* list );

#endif
