/**
 * @file  uctTree.h
 * @brief Provides types and utilities to create and update a tree of moves
 *  
 **/
#ifndef UCTTREE_H
#define UCTTREE_H

#include "board.h"
#include "memoryPool.h"

/**
 * @brief The maximum number of pools to be allocated
 **/
#define MAX_POOLS 50

/**
 * @brief UCT Statistics to be saved for every state node
 **/
typedef struct UCTNode
{
  /** Intersection to identify the move represented by this node*/
  INTERSECTION move;

  /** Wins as first move of playout */
  int winsBlack;
  /** Playouts as first move */
  int played;

  /** Wins as not first move */
  int AMAFwinsBlack;
  /** Playouts as not first move */
  int AMAFplayed;

  /** First child */
  struct UCTNode* firstChild;
  /** Next sibiling */
  struct UCTNode* nextSibiling;
  
} UCTNode;

/**
 * @brief Represents a tree of moves as UCTNodes
 *
 * The tree consists of several memory pool of UCTNodes that are
 * used to "allocate" new nodes with no overhead.  New pool are
 * dinamically allocated when all current pool memory has been used.
 * Deallocation of nodes or pools is not supported.
 **/
typedef struct UCTTree
{
  // A bunch of memory pools
  MemoryPool pools[MAX_POOLS];

  // Number of currently used pools
  int poolsNum;
  // Size in elements of a single pool
  int poolSize;

  // The root of the tree
  UCTNode root;

} UCTTree;


/**
 * @brief Macro to browse all children of a specified node
 **/
#define foreach_child(node) for(UCTNode* child=node->firstChild; child; child=child->nextSibiling)


/**
 * @brief Initializes a new tree, pre-allocation one pool of 
 * memory of the specified size.  All subsequent growth in tree
 * will be of this size.
 *
 * @brief tree The tree to initialize
 * @brief poolSize The size in elements of a single pool
 **/
void UCTTree_initialize( UCTTree* tree, int poolSize );

/**
 * @brief Release all resources related to the tree.
 * The tree and all nodes allocated from this tree
 * become unusable after deletion.
 *
 * @param tree The tree to delete
 **/
void UCTTree_delete( UCTTree* tree );

/**
 * @brief Allocates a new node for the specified tree
 *
 * @param tree The tree from which to allocate a new node.
 * The new node is initialized but not added to the tree,
 * it is caller responsibility to do so
 **/
UCTNode* UCTTree_newNode( UCTTree* tree );

/**
 * @brief Initializes a node to default values
 *
 * @param node The node to initialize
 **/
void UCTNode_initialize( UCTNode* node );

#endif
