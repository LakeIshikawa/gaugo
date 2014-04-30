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
 * @param node The node from which to descend the tree
 * @return The color of the winner of the single random playout that was performed
 **/
Color UCTSearch_playSimulation( UCTSearch* search, UCTNode* node );

/**
 * @brief Select the UCT-best children of speficied node 'pos', for the current turn player.
 *
 * @param search The search
 * @param pos The parent position
 * @return The currently UCT-best child node of specified position
 **/
UCTNode* UCTSearch_selectUCT( UCTSearch* search, UCTNode* pos );

/**
 * @brief Create all legal children position of current board state 
 * and stores them in the tree
 *
 * @param search The search
 * @param pos The position to expand
 **/
void UCTSearch_createChildren( UCTSearch* search, UCTNode* pos );

void UCTSearch_initialize( UCTSearch* search, Board* board, UCTTree* tree, 
			   POLICY policy, STOPPER stopper, Options* options )
{
  search->root = *board;
  search->tree = tree;
  search->policy = policy;
  search->stopper = stopper;
  search->options = options;
}

INTERSECTION UCTSearch_search( UCTSearch* search )
{
  // Initializes board iterator
  BoardIterator it;
  Board_iterator(&search->root, &it);

  // Prepares search data
  search->iter = &it;
  search->UCTK = 0.44f;

  int simulations = 0;
  Board boardCopy;

  // Simulate until stopper signals to stop
  do{
    // Copy the board
    Board_copy( &boardCopy, &search->root );
    search->board = &boardCopy;

    // Play one playout from the most UCT-RAVE promising node
    UCTSearch_playSimulation(search, &search->tree->root);

    simulations++;
  } while(!(*(search->stopper))(search, simulations));

  INTERSECTION pv[MAX_INTERSECTION_NUM];
  UCTSearch_getPv( search, pv, &search->tree->root );

  return pv[0];
}

void UCTSearch_getPv( UCTSearch* search, INTERSECTION* pv, UCTNode* node )
{
  // Select most visited move from root
  int mostPlayed = 0;
  INTERSECTION bestMove = PASS;
  
  // Browses all children
  foreach_child( node ){
    int played = child->played;
    if( played > mostPlayed ) {
      mostPlayed = played;
      bestMove = child->move;
    }
  }
  
  *pv = bestMove;

  // Recursion
  if( node->firstChild != NULL ){
    UCTSearch_getPv( search, pv+1, node->firstChild );
  }
}

void UCTSearch_printPv( UCTSearch* search )
{
  Board boardCopy = search->root;
  search->board = &boardCopy;

  // Gets pv
  INTERSECTION pv[MAX_INTERSECTION_NUM];
  UCTSearch_getPv( search, pv, &search->tree->root );
  
  printf("#");
  for( int i=0; i<MAX_INTERSECTION_NUM; i++ ){
    if( pv[i] == PASS ) break;
    
    char str[3];
    Board_intersectionName( search->board, pv[i], str );
    printf("%s ", str);
  }
  printf("\n");
}

Color UCTSearch_playSimulation( UCTSearch* search, UCTNode* pos )
{
  Color winner;

  // If this position is terminal, expand
  if( !pos->firstChild ){
    UCTSearch_createChildren(search, pos);

    // Play random game
    winner = (*(search->policy))(search);
  }
  else{

    // Select UCT-RAVE best node
    UCTNode* bestchild = UCTSearch_selectUCT(search, pos);

    // Go into child position
    if( bestchild->move == PASS ){
      Board_pass( search->board );
    }
    else{
      Board_play( search->board, bestchild->move );
    }

    // Recurse
    winner = UCTSearch_playSimulation( search, bestchild );
  }

  // Playout finished, update statistics
  pos->played++;

  // Update winrate
  if( winner == BLACK ) pos->winsBlack++;
  
  return winner;
}

void UCTSearch_createChildren( UCTSearch* search, UCTNode* pos )
{      
  //Board_print(search->board, stdout, 0, 0);
  //printf("Par: %016llx-%016llx\n", search->board->hashKey.key1, search->board->hashKey.key2);

  // Browses all legal children
  //HashKey child;
  UCTNode* childNode = NULL;
  foreach_empty(search->board){
    if( Board_isLegal( search->board, empty ) ){
      UCTNode* newNode = UCTTree_newNode( search->tree );
      newNode->move = empty;
      
      // Add as first child or brother of prev node
      if( childNode == NULL ) pos->firstChild = newNode;
      else childNode->nextSibiling = newNode;
      childNode = newNode;
    }
  }

  //printf("\n");
}

UCTNode* UCTSearch_selectUCT( UCTSearch* search, UCTNode* pos )
{
  float bestUCT = 0.0f;
  INTERSECTION bestMove = PASS;

  // Browses all children of current position
  HashKey child;
  UCTNode* bestChild = NULL;
  foreach_child(pos){
    float uctValue = UCTNode_evaluateUCT( child, pos, search->board->turn, search->UCTK );
    if( uctValue > bestUCT ) {
      bestUCT = uctValue;
      bestChild = child;
    }
  }

  return bestChild;
}

float UCTNode_evaluateUCT( UCTNode* node, UCTNode* parent, Color turn, float UCTK )
{
  // Random huge value for unexplored nodes
  if( node->played == 0 ) return 10000.0f + (rand()%1000);

  switch( turn ){
  case BLACK:
    return ((float)node->winsBlack / node->played) 
      + UCTK*sqrt( log(parent->played) / (5*node->played) );
  case WHITE:
    return (1.0f - ((float)node->winsBlack / node->played)) 
      + UCTK*sqrt( log(parent->played) / (5*node->played) );
  }
}
