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
  int size;
  sgfGetIntProperty( tree.lastnode, "SZ", &size );

  Board_initialize( &engine->board, size );

  // Read and play main variation
  while( sgftreeForward( &tree ) ){
    
    // Pass node
    if( is_pass_node( tree.lastnode, size ) ){
      Board_pass( &engine->board );
    }
    
    else{
      assert( is_move_node( tree.lastnode ) );
      INTERSECTION move = Board_intersection( 
					     &engine->board, 
					     get_moveY(tree.lastnode->props, size),
					     get_moveX(tree.lastnode->props, size)
					     );
      Board_play( &engine->board, move );
    }
  }
}
