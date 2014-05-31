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
#include "timer.h"

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
  // Last N board hashes for superko check in uct tree
  HashKey lastBoards[SUPERKO_HISTORY_MAX];
  int lastBoards_next;

  // Singleton-like data handles (constant along search)
  Board root;
  HashKey rootLastBoards[SUPERKO_HISTORY_MAX];
  UCTTree* tree;
  POLICY policy;
  STOPPER stopper;
  Options* options;
  Timer timer;

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
 * @param lastBoards Last boards' hash keys to avoid superko
 **/
void UCTSearch_initialize( UCTSearch* search, Board* board, UCTTree* tree, 
			   POLICY policy, STOPPER stopper, Options* options,
			   HashKey lastBoards[SUPERKO_HISTORY_MAX]);

/**
 * @brief Performs an UCT search from the specified board position, and using the
 * specified policy for playouts. The algorythm plays one playout at the time from
 * the currently UCT-RAVE most promising node, using the specified policy.
 *
 * After every playout, stopper is checked in order to determine whether to stop the search
 * or continue.
 *
 * When the search terminates, the intersection representing the best 
 * children of root position according only to the number of 
 * simulations played is returned.
 *
 **/
INTERSECTION UCTSearch_search( UCTSearch* search );

/**
 * @brief Prints the header for search info (ASCII table format)
 **/
void UCTSearch_printSearchInfoHeader();

/**
 * @brief Prints current search info to stdout (one line) in GTP comment format.
 *
 * @param search The search going on
 **/
void UCTSearch_printSearchInfo( UCTSearch* search );

/**
 * @brief Prints current search info to stderr in the form 
 * of gogui live-gfx commands, showing info about current pv
 * 
 * @param search The search going on
 **/
void UCTSearch_printSearchGoguiGfx( UCTSearch* search );
#endif
