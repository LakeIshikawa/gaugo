/**
 * @file  GTPArchiving.c
 * @brief Implementation of some GTP archiving commands
 **/

#include <assert.h>
#include "GTPArchiving.h"
#include "sgftree.h"

void GTPArchiving_loadSGF( GauGoEngine* engine, int argc, char** argv )
{
  // Must have exactly 1 argument
  if( argc != 2 ){
    GauGoEngine_sayError( UNKOWN_COMMAND );
    return;
  }

  char* filename = argv[1];

  SGFTree tree;
  tree.root = NULL;
  tree.lastnode = NULL;
  if( !sgftree_readfile( &tree, filename ) ){
    GauGoEngine_sayError( FILE_NOT_FOUND );
    return;
  }

  // Get board size and reset/resize the board accordingly
  sgftreeForward( &tree );
  int size=19;
  sgfGetIntProperty( tree.lastnode, "SZ", &size );
  engine->options.boardSize = size;
  GauGoEngine_resetBoard( engine );

  // Read and play main variation
  while( sgftreeForward( &tree ) ){
    
    // Pass node
    if( is_pass_node( tree.lastnode, size ) ){
      GauGoEngine_play( engine, PASS );
    }
    
    else{
      if( is_move_node( tree.lastnode ) ){
	INTERSECTION move = 
	  Board_intersection( 
			     engine->board, 
			     get_moveY(tree.lastnode->props, size),
			     get_moveX(tree.lastnode->props, size)
			      );
	// SGF validation check
	if( !Board_isLegal(engine->board, move) ){
	  GauGoEngine_sayError(BAD_DATA);
	}
	
	// Must be able to play
	GauGoEngine_play( engine, move );
      }
      else if( is_pass_node( tree.lastnode, size ) ){
	GauGoEngine_play( engine, PASS );
      }
    }
  }

  // Success
  GauGoEngine_saySuccess("");
}

void GTPArchiving_saveSGF( GauGoEngine* engine, int argc, char** argv )
{
  // Must have exactly 1 argument
  if( argc != 2 ){
    GauGoEngine_sayError( UNKOWN_COMMAND );
    return;
  }
  
  char* filename = argv[1];
  
  SGFTree tree;
  tree.root = NULL;
  tree.lastnode = NULL;
  sgftreeCreateHeaderNode(&tree, engine->board->size, 6.5f, 0);

  Color turn = BLACK;
  for( int m=0; m<engine->historyLength-1; m++ ) {
    INTERSECTION ints = engine->historyMoves[m];
    sgftreeAddPlay(
		   &tree, 
		   turn, 
		   Board_intersectionY(engine->board, ints), 
		   Board_intersectionX(engine->board, ints)
		   );
    turn = turn==BLACK?WHITE:BLACK;
  }
  
  writesgf(tree.root, argv[1]);

  GauGoEngine_saySuccess("");
}
