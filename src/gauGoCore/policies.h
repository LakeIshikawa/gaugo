/**
 * @file  policies.h
 * @brief Some standard playout policies
 *
 **/
#ifndef POLICIES_H
#define POLICIES_H

#include "uctSearch.h"

/**
 * @brief Pure random playout policy.
 * Non-suicide legal moves are randomly selected until 
 * no moves are left on the board.
 * Scores are then calculated using tromp-taylor rules, 
 * and the winner is returned.
 **/
Color POLICY_pureRandom( UCTSearch* search, unsigned char* playedMoves );

#endif
