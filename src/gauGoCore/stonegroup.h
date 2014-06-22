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

#include "global_defs.h"

/**
 * @brief Represents a fully connected group of stones
 **/
typedef struct StoneGroup
{
  /**
   * The sum of all (liberty vertex indexes squared)!
   **/
  int libSumSq;

  /**
   * The sum of all liberty vertex indexes
   **/
  int libSum;

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

/**
 * @brief Determines if the group is in atari
 **/
int StoneGroup_isAtari(StoneGroup* group);

/**
 * @brief Obtains the only liberty of the group
 * The group must be in atari
 **/
INTERSECTION StoneGroup_atariLiberty(StoneGroup* group);

/**
 * @brief Determines if the group is captured
 **/
int StoneGroup_isCaptured(StoneGroup* group);

/**
 * @brief Adds a liberty to the group
 **/
void StoneGroup_addLib(StoneGroup* group, INTERSECTION lib);

/**
 * @brief Subtracts a liberty from the group
 **/
void StoneGroup_subLib(StoneGroup* group, INTERSECTION lib);

#endif
