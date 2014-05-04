/**
 * @file stonegroup.h
 * @brief Defines a type used to represent a fully connected 
 * group of stones on a go board. 
 *
 * The number of stones fully connected
 * in a group can vary from one to the full board size.
 *
 **/
#ifndef STONEGROUP_H
#define STONEGROUP_H

#include "board.h"


/**
 * @brief Represents a fully connected group of stones
 **/
typedef struct StoneGroup
{
  /**
   * The number of intersections (stones) that belong to this group
   **/
  short stonesNum;

  /**
   * This group's number of liberties
   **/
  short libertiesNum;

} StoneGroup;


/**
 * @brief Utility to browse the stones of a group.
 * The current stone being browsed is set to variable 'stone'.
 *
 **/
#define foreach_stone(group) for(short i=0, stone=group->intersections[0]; i<group->stonesNum; stone=group->intersections[++i])

/**
 * @brief Utility to browse the liberties of a group.
 * The current intersection being browsed is set to variable 'liberty'.
 *
 **/
#define foreach_liberty(group) for(short i=0, liberty=group->liberties[0]; i<group->libertiesNum; liberty=group->liberties[++i])


#endif
