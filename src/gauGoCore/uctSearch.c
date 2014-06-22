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

// Move representation for solved position 
#define SOLVED 401

// Private methods 

/**
 * @brief Finds the UCT-RAVE best node in current tree, 
 * performs a playout from that node,
 * and updates hashTable with result and RAVE informations.
 *
 * @param search The search
 * @param node The node from which to descend the tree
 * @param turn Current position board's turn
 * @param depth Current tree depth
 * @param pass Comes from a pass move
 * @return The color of the winner of the single random 
 * playout that was performed
 **/
Color UCTSearch_playSimulation( UCTSearch* search, UCTNode* node, 
				unsigned char* playedMoves, Color turn,
				int depth, int pass);

/**
 * @brief Select the UCT-best children of speficied node 'pos', 
 * for the current turn player.
 *
 * @param search The search
 * @param pos The parent position
 * @param depth Current tree depth
 * @return The currently UCT-best child node of specified position
 **/
UCTNode* UCTSearch_selectUCT( UCTSearch* search, UCTNode* pos );

/**
 * @brief Create all legal children position of current board state 
 * and stores them in the tree
 *
 * @param search The search
 * @param depth Current tree depth
 * @param pos The position to expand
 **/
void UCTSearch_createChildren( UCTSearch* search, UCTNode* pos, int depth );

void UCTSearch_initialize( UCTSearch* search, Board* board, UCTTree* tree, 
			   POLICY policy, STOPPER stopper, Options* options,
			   HashKey lastBoards[SUPERKO_HISTORY_MAX])
{
  search->root = *board;
  search->tree = tree;
  search->policy = policy;
  search->stopper = stopper;
  search->options = options;
  search->lastBoards_next = 0;
  memcpy(search->rootLastBoards, lastBoards, sizeof(search->lastBoards));

  Timer_initialize( &search->timer );
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

  // Starts timer
  Timer_start( &search->timer );

  // Simulate until stopper signals to stop
  do{
    // Copy the board
    Board_copy( &boardCopy, &search->root );
    search->board = &boardCopy;
    // Copy last boards
    memcpy(search->lastBoards, search->rootLastBoards, 
	   sizeof(search->lastBoards));
    search->lastBoards_next = 0;

    // Play one playout from the most UCT-RAVE promising node
    unsigned char playedMoves[MAX_INTERSECTION_NUM];
    memset(playedMoves, 0, sizeof(playedMoves));
    UCTSearch_playSimulation(search, &search->tree->root, 
			     playedMoves, search->board->turn, 
			     0, 0);

    simulations++;
  } while(!(*(search->stopper))(search, simulations));

  INTERSECTION pv[MAX_INTERSECTION_NUM];
  UCTTree_getPv( pv, &search->tree->root );

  return pv[0];
}

void UCTSearch_printSearchInfoHeader()
{
  printf("#  %-8s %-10s %-8s %-5s %-5s   %s\n", 
	 "time", "playouts", "pps", "komi", "wr", "pv");
}

void UCTSearch_printSearchInfo( UCTSearch* search )
{
  Board boardCopy = search->root;
  search->board = &boardCopy;

  int elapsedMillis = Timer_getElapsedTime( &search->timer );
  int pps = (search->tree->root.played*1000) / elapsedMillis; 
  float wr = (float)search->tree->root.winsBlack / search->tree->root.played;
  printf("#  %-8d %-10d %-8d %-5.1f %-5.2f   ",
	 elapsedMillis, search->tree->root.played, pps, 
	 search->options->komi, wr );

  // Gets pv
  INTERSECTION pv[MAX_INTERSECTION_NUM];
  UCTTree_getPv( pv, &search->tree->root );
  
  for( int i=0; i<MAX_INTERSECTION_NUM; i++ ){
    if( !pv[i] ) break;

    char str[5];
    Board_intersectionName( search->board, pv[i], str );
    printf("%s ", str);
  }
  printf("\n");
}

void UCTSearch_printSearchGoguiGfx( UCTSearch* search )
{
  Board boardCopy = search->root;
  search->board = &boardCopy;
  
  int elapsedMillis = Timer_getElapsedTime( &search->timer );
  int pps = (search->tree->root.played*1000) / elapsedMillis; 
  float wr = (float)search->tree->root.winsBlack / search->tree->root.played;
  fprintf(stderr, "gogui-gfx: TEXT %-3.2fs %dpo %dpps %-3.1fk %-4.2fwr\n",
	 (float)elapsedMillis/1000.0f, search->tree->root.played, pps, 
	 search->options->komi, wr );

  // Gets pv
  INTERSECTION pv[MAX_INTERSECTION_NUM];
  UCTTree_getPv( pv, &search->tree->root );
    
  // VAR
  fprintf(stderr, "gogui-gfx: VAR ");
  Color turn = boardCopy.turn;
  for( int i=0; i<MAX_INTERSECTION_NUM; i++ ){
    if( !pv[i] ) break;
    
    char str[5];
    Board_intersectionName( search->board, pv[i], str );
    fprintf(stderr, "%c %s ", turn==BLACK?'b':'w', str);

    turn = !turn;
  }
  
  fprintf(stderr, "\n");  
}

Color UCTSearch_playSimulation( UCTSearch* search, UCTNode* pos, 
				unsigned char* playedMoves, Color turn,
				int depth, int pass )
{
  Color winner;
  
  // If this position is terminal, expand
  if( pos->played < search->options->expansionVisits ){
    if( !pos->firstChild ){
      UCTSearch_createChildren(search, pos, depth);
    }

    // Play random game
    winner = (*(search->policy))(search->board, search->iter, 
				 search->options->komi, playedMoves);
  }
  else{

    // Select UCT-RAVE best node
    UCTNode* bestchild = UCTSearch_selectUCT(search, pos);

    // Go into child position
    if( bestchild->move == PASS ){
      if( pass ){
	// Node is solved!
	int score = (float)Board_trompTaylorScore( search->board, search->iter );
	winner = (score > search->options->komi) ? BLACK : WHITE;
	pos->played++;
	if( winner == BLACK ) pos->winsBlack++;
	return winner;
      }

      pass = 1;
      Board_pass( search->board );
    }
    else{
      pass = 0;
      Board_play( search->board, bestchild->move );
    }

    // Save board hash to last boards (superko check)
    search->lastBoards[search->lastBoards_next] = search->board->hashKey;
    search->lastBoards_next = (search->lastBoards_next+1) % SUPERKO_HISTORY_MAX;

    // Recurse
    winner = UCTSearch_playSimulation( search, bestchild, 
				       playedMoves, !turn, depth+1, pass );
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

void UCTSearch_createChildren( UCTSearch* search, UCTNode* pos, int depth )
{      
  // Browses all legal children
  UCTNode* childNode = NULL;
  int empty;
  int numChildren = 0;
  for(EMPTIES(search->board)){
    empty = EMPTYI(search->board);

    if( Board_isLegalNoEyeFilling( search->board, empty ) ){

      // Superko check
      HashKey childHash = Board_childHash( search->board, i );
      int superko = 0;
      for(int k=0; k<SUPERKO_HISTORY_MAX; k++){
	if( childHash == search->lastBoards[k] ) {
	  superko = 1;
	  break;
	}
      }
      if( superko ) continue;

      UCTNode* newNode = UCTTree_newNode( search->tree );
      newNode->move = empty;

      // Add as first child or brother of prev node
      if( childNode == NULL ) pos->firstChild = newNode;
      else childNode->nextSibiling = newNode;
      childNode = newNode;
      numChildren++;
    }
  }

        
  // Create pass node
  if( numChildren <= UCT_PASSNODE_MAX_CHILDREN ){
    UCTNode* newNode = UCTTree_newNode( search->tree );
    newNode->move = PASS;
    if( childNode == NULL ) pos->firstChild = newNode;
    else childNode->nextSibiling = newNode;
  }
}

UCTNode* UCTSearch_selectUCT( UCTSearch* search, UCTNode* pos )
{
  float bestUCT = -100.0f;
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

float UCTNode_evaluateUCT( const UCTNode* node, const UCTNode* parent, 
			   Color turn, float UCTK )
{
  // Random huge value for unexplored nodes
  float amaf = ((float)node->AMAFwinsBlack / (node->AMAFplayed+1) );
  if( node->played == 0 ) 
    return 10000.0f + ((turn==BLACK) ? amaf : 1.0f-amaf);

  // AMAF weight
  float beta = sqrt(500.0/(3*node->played+500));
  float value;
  float uct = UCTK*sqrt( log(parent->played) / (5*node->played) );

  switch( turn ){
  case BLACK:
    value = (((float)node->winsBlack / node->played));
    break;
    
  case WHITE:
    value = (1.0f - ((float)node->winsBlack / node->played));
    amaf = 1.0f - amaf;
    break;

  default:
    amaf = -1;
    value = -1;
    gauAssert(0, NULL, NULL);
  }

  return (1-beta)*value + beta*amaf + uct;
}
