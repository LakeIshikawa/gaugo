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
 * @brief Finds the UCT-RAVE best node in current tree, 
 * performs a playout from that node,
 * and updates hashTable with result and RAVE informations.
 *
 * @param search The search
 * @param node The node from which to descend the tree
 * @param turn Current position board's turn
 * @return The color of the winner of the single random 
 * playout that was performed
 **/
Color UCTSearch_playSimulation( UCTSearch* search, UCTNode* node, 
				unsigned char* playedMoves, Color turn );

/**
 * @brief Select the UCT-best children of speficied node 'pos', 
 * for the current turn player.
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
    unsigned char playedMoves[MAX_INTERSECTION_NUM];
    memset(playedMoves, 0, sizeof(playedMoves));
    UCTSearch_playSimulation(search, &search->tree->root, 
			     playedMoves, search->board->turn);

    simulations++;
  } while(!(*(search->stopper))(search, simulations));

  INTERSECTION pv[MAX_INTERSECTION_NUM];
  UCTSearch_getPv( search, pv, &search->tree->root );

  return pv[0];
}

void UCTSearch_getPv( UCTSearch* search, INTERSECTION* pv, UCTNode* node )
{
  // Select most visited move from root
  int mostPlayed = -1;
  INTERSECTION bestMove = PASS;
  UCTNode* bestChild = NULL;

  // Browses all children
  foreach_child( node ){
    int played = child->played;
    if( played > mostPlayed ) {
      mostPlayed = played;
      bestMove = child->move;
      bestChild = child;
    }
  }
  
  *pv = bestMove;

  // Recursion
  if( node->firstChild != NULL ){
    UCTSearch_getPv( search, pv+1, bestChild );
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
    
    char str[5];
    Board_intersectionName( search->board, pv[i], str );
    printf("%s ", str);
  }
  printf("\n");
}

Color UCTSearch_playSimulation( UCTSearch* search, UCTNode* pos, 
				unsigned char* playedMoves, Color turn )
{
  Color winner;

  // If this position is terminal, expand
  if( pos->played < 2 ){
    if( !pos->firstChild ){
      UCTSearch_createChildren(search, pos);
    }

    // Play random game
    winner = (*(search->policy))(search, playedMoves);
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
    winner = UCTSearch_playSimulation( search, bestchild, 
				       playedMoves, !turn );
  }

  // Playout finished, update statistics
  pos->played++;
  // Update winrate
  if( winner == BLACK ) {
    pos->winsBlack++;
  }

  // Update AMAF (sibilings)
  foreach_child(pos){
    if( playedMoves[child->move] & (turn+1) ){
      child->AMAFplayed++;
      if( winner == BLACK ) child->AMAFwinsBlack++;
    }
  }

  return winner;
}

void UCTSearch_createChildren( UCTSearch* search, UCTNode* pos )
{      
  // Browses all legal children
  UCTNode* childNode = NULL;
  int empty;
  for(EMPTIES(search->board)){
    empty = EMPTYI(search->board);

    if( Board_isLegalNoEyeFilling( search->board, empty ) ){
      UCTNode* newNode = UCTTree_newNode( search->tree );
      newNode->move = empty;

      // Add as first child or brother of prev node
      if( childNode == NULL ) pos->firstChild = newNode;
      else childNode->nextSibiling = newNode;
      childNode = newNode;
    }
  }
}

UCTNode* UCTSearch_selectUCT( UCTSearch* search, UCTNode* pos )
{
  float bestUCT = 0.0f;
  INTERSECTION bestMove = PASS;

  // Browses all children of current position
  UCTNode* bestChild = NULL;
  foreach_child(pos){
    float uctValue = UCTNode_evaluateUCT( 
					 child, 
					 pos, 
					 search->board->turn, 
					 search->UCTK );
    if( uctValue > bestUCT ) {
      bestUCT = uctValue;
      bestChild = child;
    }
  }

  return bestChild;
}

float UCTNode_evaluateUCT( UCTNode* node, UCTNode* parent, 
			   Color turn, float UCTK )
{
  // Random huge value for unexplored nodes
  if( node->played == 0 ) return 10000.0f + (rand()%1000);

  // AMAF weight
  float beta = sqrt(500.0/(3*node->played+500));
  float value, amaf;
  float uct = UCTK*sqrt( log(parent->played) / (5*node->played) );

  switch( turn ){
  case BLACK:
    value = (((float)node->winsBlack / node->played));
    amaf = ((float)node->AMAFwinsBlack / node->AMAFplayed);
    break;
    
  case WHITE:
    value = (1.0f - ((float)node->winsBlack / node->played));
    amaf = (1.0f - (float)node->AMAFwinsBlack / node->AMAFplayed);
    break;

  default:
    amaf = -1;
    value = -1;
    gauAssert(0, NULL, NULL);
  }

  return (1-beta)*value + beta*amaf + uct;
}
