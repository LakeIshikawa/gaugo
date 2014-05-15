/**
 * @file  options.h
 * @brief GauGo GTP engine options
 *
 **/
#ifndef OPTIONS_H
#define OPTIONS_H


/**
 * @brief GauGo program options
 *
 **/
typedef struct Options
{
  /** Board size */
  int boardSize;

  /** Hash table size (bits) */
  int hashTableSize;

  /** Number of nodes for a single pool of nodes in tree */
  int treePoolNodeNum;

  /** komi */
  float komi;

  /** verbosity: 0=GTP only 1=genmove log 2=log all */
  int verbosity;

} Options;


void Options_initialize( Options* options, int argc, char** argv );

#endif
