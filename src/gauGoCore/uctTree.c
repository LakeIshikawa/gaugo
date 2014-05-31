/**
 * @file uctTree.c
 * @brief UCT tree implementation
 *
 **/
#include "uctTree.h"
#include "crash.h"
#include "string.h"

/**
 * @brief Allocates a supplementary pool of memory for the tree,
 * or fail if no more memory is available.
 *
 **/
int UCTTree_grow( UCTTree* tree )
{
  if( tree->poolsNum >= MAX_POOLS ) return 0; 
  return MemoryPool_initialize(
			       &tree->pools[tree->poolsNum++], 
			       tree->poolSize, 
			       sizeof(UCTNode));
}


void UCTTree_initialize( UCTTree* tree, int poolSize, Board* board )
{
  UCTTree_initializeEmpty( tree );
  tree->poolSize = poolSize;
  memcpy(&tree->rootHash, &board->hashKey, sizeof(HashKey));
  UCTTree_grow( tree );
}

void UCTTree_initializeEmpty( UCTTree* tree )
{
  tree->poolsNum = 0;
  UCTNode_initialize(&tree->root);
  memset(&tree->rootHash, 0, sizeof(HashKey));
}

void UCTTree_delete( UCTTree* tree )
{
  for( int i=0; i<tree->poolsNum; i++ ){
    MemoryPool_delete( &tree->pools[i] );
  }
}

UCTNode* UCTTree_newNode( UCTTree* tree )
{
  UCTNode* newNode = MemoryPool_allocate( &tree->pools[tree->poolsNum-1] );
  
  // If allocation failed, we need another pool
  if( newNode == NULL ){
    if( !UCTTree_grow( tree ) ) return NULL;
    
    // Reallocate with new pool
    newNode = MemoryPool_allocate( &tree->pools[tree->poolsNum-1] );
  }

  // It is mandatory that allocation succeeded (cannot be out of memory here)
  gauAssert(newNode!=NULL, NULL, NULL);
  return newNode;
}

void UCTTree_getPv( INTERSECTION* pv, UCTNode* node )
{
  // Select most visited move from root
  int mostPlayed = -1;
  INTERSECTION bestMove = 0;
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
    UCTTree_getPv( pv+1, bestChild );
  }
}

void UCTNode_initialize( UCTNode* node )
{
  memset( node, 0, sizeof(UCTNode) );
}

