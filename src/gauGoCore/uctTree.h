/**
 * @file  uctTree.h
 * @brief Provides types and utilities to create and update a tree of moves
 *  
 **/
#ifndef UCTTREE_H
#define UCTTREE_H

#include "board.h"
#include "hashTable.h"
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
  /** First child */
  struct UCTNode* firstChild;
  /** Next sibiling */
  struct UCTNode* nextSibiling;

  /** Wins as first move of playout */
  int winsBlack;
  /** Playouts as first move */
  int played;

  /** Wins as not first move */
  int AMAFwinsBlack;
  /** Playouts as not first move */
  int AMAFplayed;
  
  /** Intersection to identify the move represented by this node*/
  INTERSECTION move;
  
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
  // The root board hash (the position this tree belongs to)
  HashKey rootHash;

} UCTTree;


/**
 * @brief Macro to browse all children of a specified node
 **/
#define foreach_child(node) for(UCTNode* child=node->firstChild; child; child=child->nextSibiling)

/**
 * @brief Macro to browse all sibilings of a specified node
 **/
#define foreach_sibiling(node) for(UCTNode* bro=node->nextSibiling; bro; bro=bro->nextSibiling)


/**
 * @brief Initializes a new tree, pre-allocation one pool of 
 * memory of the specified size.  All subsequent growth in tree
 * will be of this size.
 *
 * @brief tree The tree to initialize
 * @brief poolSize The size in elements of a single pool
 * @brief rootPos The root's board position (the hash key will be stored)
 **/
void UCTTree_initialize( UCTTree* tree, int poolSize, Board* rootPos );

/**
 * @brief Initializes without allocating a pool size (for using as an empty tree)
 * No nodes can be added to this tree until UCTTree_delete, UCTTree_initialize
 * are called
 *
 * @param tree The tree
 **/
void UCTTree_initializeEmpty( UCTTree* tree );

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
 * @brief Writes the resulting pv to the specified array.
 *
 * @param pv The array to which pv moves will be written starting at position 0. 
 * All positions past last move in table are filled with value PASS.
 * @param The node from which to get the pv
 **/
void UCTTree_getPv( INTERSECTION* pv, UCTNode* node );

/**
 * @brief Initializes a node to default values
 *
 * @param node The node to initialize
 **/
void UCTNode_initialize( UCTNode* node );

/**
 * @brief Evaluate a node based on its UCT-RAVE values 
 * (UCB standard formula extended with RAVE)
 * A node with no direct visits is always given maximum value
 *
 * @param node The node to evaluate UCT value
 * @param parent The parent of 'node'
 * @param turn Turn color at node
 * @param UCTK UCT constant
 **/
float UCTNode_evaluateUCT( const UCTNode* node, const UCTNode* parent, 
			   Color turn, float UCTK);

#endif
