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
  options->verbosity = 1;
  options->expansionVisits = 7;
  options->gogui = 0;

  // Parse command line options
  static struct option long_options[] = {
    {"boardsize", required_argument, 0,  's'},
    {"hashsize", required_argument, 0,  'h'},
    {"tpnodes", required_argument, 0, 'p'},
    {"komi", required_argument, 0,  'k'},
    {"verbosity", required_argument, 0, 'v'},
    {"expansion_visits", required_argument, 0, 'x'},
    {"gogui", no_argument, 0, 'g'},
    //{"datapath", required_argument, 0,  'd' },
    {0,         0,                 0,  0 }
  };
  int c, option_index;
  
  while(1){
    c = getopt_long(argc, argv, "s:h:k:p:v:x:g:", long_options, &option_index);
    if( c==-1 ) break;
    switch(c){
      // Board size
    case 's' : options->boardSize = atoi(optarg); break;
      // Hash table size bits
    case 'h' : options->hashTableSize = atoi(optarg); break;
      // Nodes in one pool in tree
    case 'p' : options->treePoolNodeNum = atoi(optarg); break;
      // Komi
    case 'k' : options->boardSize = atof(optarg); break;
      // Verbosity
    case 'v' : options->verbosity = atoi(optarg); break;
      // Expansion visits threashold
    case 'x' : options->expansionVisits = atoi(optarg); break;
      // Gogui output mode
    case 'g': options->gogui = 1; break;
    }
  }
}
