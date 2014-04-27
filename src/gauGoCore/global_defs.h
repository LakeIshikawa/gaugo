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
 * @brief The maximum number of needed intersection data
 * in order to represent a go board of size MAX_BOARD_SIZE.
 * The first and last line of the board will be filled with BORDER
 * intersections, as long as the last column.
 **/
#define MAX_INTERSECTION_NUM ((MAX_BOARD_SIZE+1) * (MAX_BOARD_SIZE+2))

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
#define MAX_STONEGROUPS 256


#endif
