/**
 * @file stonegroup.c
 * @brief StoneGroup implementation
 *
 **/

#include "stonegroup.h"
#include "squares.h"
#include "crash.h"

/**
 * @brief Determines if the group is in atari
 **/
int StoneGroup_isAtari(StoneGroup* group)
{
  /* This is pure genius (thanks libego)
   * 3(a^2+b^2+c^2) == (a+b+c)^2 IFF a==b==c, 
   * because for ex: 
   * (a+a+a)^2 -> 6a^2 and 3*(a^2*a^2*a^2) -> 6a^2
   *
   * More generally:
   * n*(a1^2+..+an^2) == (a1+..+an)^2 IFF a1==...==an
   * 
   * So we have n liberties including multiple ones,
   * that are added more than once if they are shared
   * by 2 merging groups (i.e. they are neighs of more than
   * 1 stone belonging to the group) and they are also
   * subtracted more than once when a stone is played in
   * their intersection, so either they are present 
   * multiple times in the sum or they arent present at all.
   *
   * And we have the incremental sum for liberties' indexes 
   * and the one for liberties's indexes squares
   *
   * So the only case of atari is when 
   * n*(l1^2+...+ln^2) == (l1+...+ln)^2
   * but that is
   * n*(libSumSquared) == (libSum)^2
   *
   * because that means all terms of libSum are
   * equal, which means they are all the same vertex,
   * and were added multiple times!!
   * 
   * Just wow.
   */
  return group->libertiesNum * group->libSumSq == 
    group->libSum * group->libSum;
}

INTERSECTION StoneGroup_atariLiberty(StoneGroup* group)
{
  gauAssert(group->libSum % group->libertiesNum == 0, NULL, NULL);
  return group->libSum / group->libertiesNum;
}

int StoneGroup_isCaptured(StoneGroup* group)
{
  return group->libertiesNum == 0;
}

/**
 * @brief Adds a liberty to the group
 **/
void StoneGroup_addLib(StoneGroup* group, INTERSECTION lib)
{
  group->libertiesNum++;
  group->libSum += lib;
  group->libSumSq += intSquared[lib];
}

/**
 * @brief Subtracts a liberty from the group
 **/
void StoneGroup_subLib(StoneGroup* group, INTERSECTION lib)
{
  group->libertiesNum--;
  group->libSum -= lib;
  group->libSumSq -= intSquared[lib];  
}
