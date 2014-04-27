/**
 * @file  stoppers.h
 * @brief Some standard uct search stoppers
 *
 **/
#ifndef STOPPERS_H
#define STOPPERS_H

#include "uctSearch.h"

/**
 * @brief Default stopper policy with 5k simulations
 **/
int STOPPER_5ksim( UCTSearch* search, int simulations );

#endif
