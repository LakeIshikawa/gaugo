/**
 * @file uctSearch.c
 * @brief UCT search implementation
 *
 **/
#include "uctSearch.h"
#include "crash.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/**
 * A new node with no children
 **/
UCTNode NEW_NODE = { 0,0,0,0 };

// Private methods 

/**
 * @brief Evaluate a node based on its UCT-RAVE values (UCB standard formula extended with RAVE)
 * A node with no direct visits is always given maximum value
 **/
float UCTNode_evaluateUCT( UCTNode* node, UCTNode* parent, Color turn, float UCTK);

/**
 * @brief Finds the UCT-RAVE best node in current tree, performs a playout from that node,
 * and updates hashTable with result and RAVE informations.
 *
 * @param search The search
 * @return The color of the winner of the single random playout that was performed
 **/
Color UCTSearch_playSimulation( UCTSearch* search );

/**
 * @brief Select the UCT-best children of speficied node 'pos', for the current turn player.
 *
 * @param search The search
 * @param pos The parent position
 **/
INTERSECTION UCTSearch_selectUCT( UCTSearch* search, UCTNode* pos );

/**
 * @brief Create all legal children position of current board state 
 * initializing them to NEW_NODE, and stores them in the hash table.
 *
 **/
void UCTSearch_createChildren( UCTSearch* search );

void UCTSearch_initialize( UCTSearch* search, Board* board, HashTable* hashTable, POLICY policy, STOPPER stopper, Options* options )
{
  search->root = *board;
  search->hashTable = hashTable;
  search->policy = policy;
  search->stopper = stopper;
  search->options = options;
}

INTERSECTION UCTSearch_search( UCTSearch* search )
{
  // Initializes board iterator
  BoardIterator it;
  Board_iterator(&search->root, &it);

  // Initializes empty square list
  EmptiesList emptiesList;

  // Prepares search data
  search->iter = &it;
  search->emptiesList = &emptiesList;
  search->UCTK = 0.44f;

  int simulations = 0;
  Board boardCopy;

  // Simulate until stopper signals to stop
  do{
    // Copy the board
    Board_copy( &boardCopy, &search->root );
    search->board = &boardCopy;

    // Rebuild empties list
    EmptiesList_initialize( &emptiesList, &it, &boardCopy);

    // Play one playout from the most UCT-RAVE promising node
    UCTSearch_playSimulation(search);

    simulations++;
  } while(!(*(search->stopper))(search, simulations));

  // Obtains pv
  boardCopy = search->root;
  search->board = &boardCopy;

  INTERSECTION pv[MAX_INTERSECTION_NUM];
  UCTSearch_getPv( search, pv );

  return pv[0];
}

void UCTSearch_getPv( UCTSearch* search, INTERSECTION* pv )
{
  // Select most visited move from root
  int mostPlayed = 0;
  INTERSECTION bestMove = PASS;
  
  // Browses all legal children
  HashKey child;
  UCTNode* childNode;
  BoardIterator* iter = search->iter;
  foreach_intersection(iter){
    if( Board_isLegal( search->board, intersection ) ){
      Board_childHash( search->board, intersection, &child );
      
      // Probe child
      childNode = HashTable_retrieve( search->hashTable, &child );
      if( !childNode ) continue;

      int played = childNode->played;
      if( played > mostPlayed ) {
	mostPlayed = played;
	bestMove = intersection;
      }
    }
  }

  *pv = bestMove;

  // Recursion
  if( bestMove != PASS ){
    Board_play( search->board, bestMove );
    UCTSearch_getPv( search, pv+1 );
  }
}

void UCTSearch_printPv( UCTSearch* search )
{
  Board boardCopy = search->root;
  search->board = &boardCopy;

  // Gets pv
  INTERSECTION pv[MAX_INTERSECTION_NUM];
  UCTSearch_getPv( search, pv );
  
  printf("#");
  for( int i=0; i<MAX_INTERSECTION_NUM; i++ ){
    if( pv[i] == PASS ) break;
    
    char str[3];
    Board_intersectionName( search->board, pv[i], str );
    printf("%s ", str);
  }
  printf("\n");
}

Color UCTSearch_playSimulation( UCTSearch* search )
{
  Color winner;

  // Probe current position
  UCTNode* pos = HashTable_retrieve( search->hashTable, &search->board->hashKey );
  if( !pos || pos->played == 0){
    // Store the position and expand
    if( !pos ){
      pos = HashTable_insert( search->hashTable, &search->board->hashKey, &NEW_NODE );
    }
    UCTSearch_createChildren(search);

    // Play random game
    winner = (*(search->policy))(search);
  }
  else{

    // Select UCT-RAVE best node
    INTERSECTION move = UCTSearch_selectUCT(search, pos);

    // Go into child position
    if( move == PASS ){
      Board_pass( search->board );
    }
    else{
      Board_playUpdatingEmpties( search->board, move, search->emptiesList );
    }

    // Recurse
    winner = UCTSearch_playSimulation( search );
  }

  // Playout finished, update statistics
  pos->played++;

  // Update winrate
  if( winner == BLACK ) pos->winsBlack++;
  
  return winner;
}

void UCTSearch_createChildren( UCTSearch* search )
{      
  //Board_print(search->board, stdout, 0, 0);
  //printf("Par: %016llx-%016llx\n", search->board->hashKey.key1, search->board->hashKey.key2);

  // Browses all legal children
  HashKey child;
  UCTNode* childNode;
  foreach_empty(search->emptiesList){
    if( Board_isLegal( search->board, emptyNode->intersection ) ){
      Board_childHash( search->board, emptyNode->intersection, &child );

      // Probe child
      childNode = HashTable_retrieve( search->hashTable, &child );
      if( !childNode ){
	childNode = HashTable_insert( search->hashTable, &child, &NEW_NODE );
      }

      // KILL ME
      //printf("%d(%016llx-%016llx) ", emptyNode->intersection, child.key1, child.key2);
    }
  }

  //printf("\n");
}

INTERSECTION UCTSearch_selectUCT( UCTSearch* search, UCTNode* pos )
{
  float bestUCT = 0.0f;
  INTERSECTION bestMove = PASS;

  // Browses all legal children
  HashKey child;
  UCTNode* childNode;
  foreach_intersection(search->iter){
    if( Board_isLegal( search->board, intersection ) ){
      Board_childHash( search->board, intersection, &child );
      
      // Probe child
      childNode = HashTable_retrieve( search->hashTable, &child );
      
      gauAssertMsg(childNode!=NULL, search->board, search,
		   "Child not found:%d(%016llx-%016llx)\n", intersection, child.key1, child.key2);

      float uctValue = UCTNode_evaluateUCT( childNode, pos, search->board->turn, search->UCTK );
      if( uctValue > bestUCT ) {
	bestUCT = uctValue;
	bestMove = intersection;
      }
    }
  }

  return bestMove;
}

float UCTNode_evaluateUCT( UCTNode* node, UCTNode* parent, Color turn, float UCTK )
{
  // Random huge value for unexplored nodes
  if( node->played == 0 ) return 10000.0f + (rand()%1000);

  switch( turn ){
  case BLACK:
    return ((float)node->winsBlack / node->played) + UCTK*sqrt( log(parent->played) / (5*node->played) );
  case WHITE:
    return (1.0f - ((float)node->winsBlack / node->played)) + UCTK*sqrt( log(parent->played) / (5*node->played) );
  }
}
