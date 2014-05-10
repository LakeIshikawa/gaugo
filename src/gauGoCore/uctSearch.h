/**
 * @file  uctSearch.h
 * @brief Provides a modular implementation of the UCT algorythm
 * applied to a go board.
 *
 * The UCT algorythm rely on a pluggable playout-policy that determines
 * the way random-play is performed.
 *  
 **/
#ifndef UCTSEARCH_H
#define UCTSEARCH_H

#include "global_defs.h"
#include "board.h"
#include "hashTable.h"
#include "options.h"
#include "uctTree.h"

#include <stdio.h>

struct UCTSearch;

/**
 * @brief Function that transform an arbitrary go board 
 * into a finished game (i.e. performs a playout)
 **/
typedef Color (*POLICY)(struct UCTSearch*, unsigned char* playedMoves);

/**
 * @brief Function that determines whether to stop UCT 
 * search or not (The number of total playouts 
 * performed is provided by callback's function first argument)
 **/
typedef int (*STOPPER)(struct UCTSearch*, int);


/**
 * @brief Data used inside an UCT search (packed to avoid 
 * passing large data as function parameters)
 **/
typedef struct UCTSearch
{
  // Incremental/temporary handles (change along search)
  Board* board;
  BoardIterator* iter;

  // Singleton-like data handles (constant along search)
  Board root;
  UCTTree* tree;
  POLICY policy;
  STOPPER stopper;
  Options* options;

  // UCT exploration/exploitation parameter
  float UCTK;

} UCTSearch;

/**
 * @brief Initializes uct search data with provided parameters
 *
 * @param search The search to initialize
 * @param board The board position
 * @param tree An initialized UCT tree (can be non-empty, care for the size though)
 * @param policy A playout policy
 * @param stopper Function to stop the search arbitrarily
 * @param options Search options
 **/
void UCTSearch_initialize( UCTSearch* search, Board* board, UCTTree* tree, 
			   POLICY policy, STOPPER stopper, Options* options );

/**
 * @brief Performs an UCT search from the specified board position, and using the
 * specified policy for playouts. The algorythm plays one playout at the time from
 * the currently UCT-RAVE most promising node, using the specified policy.
 *
 * After every playout, stopper is checked in order to determine whether to stop the search
 * or continue.
 *
 * When the search terminates, the intersection representing the best 
 * children of root position according
 * only to winrate is returned.  For further informations, consult the hash table.
 *
 **/
INTERSECTION UCTSearch_search( UCTSearch* search );

/**
 * @brief Writes the resulting pv to the specified array.
 *
 * @param search The search
 * @param pv The array to which pv moves will be written starting at position 0. All positions
 * past last move in table are filled with value PASS.
 * @param The node from which to get the pv
 **/
void UCTSearch_getPv( UCTSearch* search, INTERSECTION* pv, UCTNode* node );

/**
 * @brief Prints current pv to stdout in GTP comment format.
 *
 * @param search The search going on
 **/
void UCTSearch_printPv( UCTSearch* search );
#endif
