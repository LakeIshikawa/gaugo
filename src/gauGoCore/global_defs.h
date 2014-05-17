/**
 * @file global_defs.h
 * @brief Header file that contains definitions that are global to \
 * the gauGoCore module.
 *
 **/ 
#ifndef GLOBAL_DEFS_H
#define GLOBAL_DEFS_H


/**
 * @brief Minimum possible board size
 **/
#define MIN_BOARD_SIZE 4

/**
 * @brief Maximum possible board size
 **/
#define MAX_BOARD_SIZE 19

/**
 * @brief Maximum game length that
 * the engine can support
 **/
#define HISTORY_LENGTH_MAX 512

/**
 * @brief Max number of previous board states
 * to be checked for super-ko inside tree search
 **/
#define SUPERKO_HISTORY_MAX 8

/**
 * @brief The maximum number of needed intersection data
 * in order to represent a go board of size MAX_BOARD_SIZE.
 * The first and last line of the board will be filled with BORDER
 * intersections, as long as the last column.
 **/
#define MAX_INTERSECTION_NUM ((MAX_BOARD_SIZE+1) * (MAX_BOARD_SIZE+2))

/**
 * @brief Maximum number of moves in one playout 
 * (to avoid superko infinite games)
 **/
#define PLAYOUT_MOVES_MAX 512

/**
 * @brief Type used to index stone groups in the stone group pool
 **/
typedef unsigned char GRID;

/**
 * @brief Type used to represent intersections
 **/
typedef short INTERSECTION;

/**
 * @brief Constant representing the null stone group 
 * (= no stone group informations)
 **/
#define NULL_GROUP ((GRID)255)

/**
 * @brief Maximum supported number of groups on the board.
 * The maximum theoretical number of legal strings(groups) is
 * actually 277... whatever.
 **/
#define MAX_STONEGROUPS 255

/**
 * @brief The maximum number of children a node can have for
 * the pass move to be created.  No pass move will be added to 
 * one node's children if the children number of that node
 * excluding the pass move if above this value
 *
 **/
#define UCT_PASSNODE_MAX_CHILDREN 10

/**
 * @brief Some value close to infinity
 **/
#define INFINITY 0xFFFFFFF

#endif
