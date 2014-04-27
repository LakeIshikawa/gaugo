/**
 * @file  board_zobrist.h
 * @brief Provides the set of random values to use for Zobrist hashing
 * on the go board.
 *
 * Every intersection of the board is given 3 values: one for the state 
 * in which a black stone is on the intersection, one for a white stone, 
 * and one for a ko status.
 * No values are given to empty intersection, so the hash value of an empty 
 * go board is 0.
 *  
 **/
#ifndef BOARD_ZOBRIST_H
#define BOARD_ZOBRIST_H

#include "global_defs.h"

/**
 * @brief Zobrist values set for a go board
 **/
typedef struct ZobristValues
{
  unsigned long long black[MAX_INTERSECTION_NUM];
  unsigned long long white[MAX_INTERSECTION_NUM];
  unsigned long long ko[MAX_INTERSECTION_NUM];
  unsigned long long turn;
} ZobristValues;


/**
 * @brief Initialized a set of random values for Zobrist hashing
 **/
void ZobristValues_initialize(ZobristValues* zobristValues, unsigned int seed);

#endif
