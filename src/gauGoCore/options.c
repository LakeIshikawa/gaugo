/**
 * @file  options.c
 * @brief GauGo GTP engine options
 *
 **/

#include <getopt.h>
#include <stdlib.h>
#include "options.h"

void Options_initialize( Options* options, int argc, char** argv )
{ 
  // First set all defaults
  options->boardSize = 9;
  options->hashTableSize = 24;
  options->treePoolNodeNum = 1000000;
  options->komi = 7.5f;

  // Parse command line options
  static struct option long_options[] = {
    {"boardsize", no_argument, 0,  's'},
    {"hashsize", no_argument, 0,  'h'},
    {"tpnodes", no_argument, 0, 'p'},
    {"komi", no_argument, 0,  'k'},
    //{"datapath", required_argument, 0,  'd' },
    {0,         0,                 0,  0 }
  };
  int c, option_index;
  
  while(1){
    c = getopt_long(argc, argv, "s:h:k:p:", long_options, &option_index);
    if( c==-1 ) break;
    switch(c){
    case 's' : options->boardSize = atoi(optarg); break;           // Board size
    case 'h' : options->hashTableSize = atoi(optarg); break;       // Hash table size bits
    case 'p' : options->treePoolNodeNum = atoi(optarg); break;     // Nodes in one pool in tree
    case 'k' : options->boardSize = atof(optarg); break;           // Board size
    }
  }
}
