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

  /**
   * Points to the intersection which is the head of the group 
   * i.e. the first stone placed on the board for this group
   **/
  INTERSECTION groupHead;

} StoneGroup;


/**
 * @brief Utility to browse the stones of a group.
 * The current stone being browsed is set to variable 'stone'.
 *
 **/
#define STONES(board, group) INTERSECTION _st=board->groups[group].groupHead; _st; _st=board->nextStone[_st]
#define STONEI() _st

#endif
