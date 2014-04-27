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

  /** komi */
  float komi;

} Options;


void Options_initialize( Options* options, int argc, char** argv );

#endif
