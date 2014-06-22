/**
 * @file  features.h
 * @brief Provides functionality for computing some heuristic
 * features on legal moves of a board position.
 *
 * All state informations are implemented inside the board,
 * this file only extends board functionality to check whether
 * features are present for some legal move.  
 **/
#ifndef FEATURES_H
#define FEATURES_H

#include "board.h"

/**
 * @brief Checks for capture heuristics
 * Capture feature types:
 * 1: The move captures a string contiguous to a string in new atari
 * 2: The move re-captures the previous move
 * 3: The move prevents a connection to the previous move (i.e. it is
 * in the middle of 2 opponent stones one of which is the previous move
 *
 * @return 0 if no feature is detected, otherwise 
 * the rep number of that feature
 **/
int Features_capture(Board* board, INTERSECTION move);

/**
 * @brief Checks for extension heuristic
 * Extension feature types:
 * 1: The move is an extension of a string in atari
 *
 * @return 0 if no feature is detected, otherwise 
 * the rep number of that feature
 **/
int Features_extension(Board* board, INTERSECTION move);

/**
 * @brief Checks for self-atari heuristic
 * Self-atari feature types:
 * 1: The move is self-atari
 * 
 * @return 0 if no feature is detected, otherwise 
 * the rep number of that feature
 **/
int Features_selfAtari(Board* board, INTERSECTION move);

/**
 * @brief Checks for atari heuristic
 * Atari heuristic types:
 * 1: The move is an atari and there is a ko
 * 2: The move is atari and there is no ko
 *
 * @return 0 if no feature is detected, otherwise 
 * the rep number of that feature
 **/
int Features_atari(Board* board, INTERSECTION move);

/**
 * @brief Checks for previous-move distance heuristics
 * Previous move distance types:
 * 2: The move is an extension
 * 3: The move is a diagonal
 *
 * @return 0 if no feature is detected, otherwise 
 * the rep number of that feature
 **/
int Features_distancePrevious(Board* board, INTERSECTION move);

#endif
