/**
 * @file  stoppers.c
 * @brief Some standard uct search stoppers implementations
 *
 **/

#include "stoppers.h"

void printTreeInfo( UCTSearch* search )
{
}

int STOPPER_5ksim( UCTSearch* search, int simulations )
{
  if( simulations % 3000 == 0 ){
    
    // Prints pv
    if( search->options->verbosity > 0 ){
      // Header
      if( simulations == 3000 ){
	UCTSearch_printSearchInfoHeader();
      }
      // Line
      UCTSearch_printSearchInfo( search );
      fflush(stdout);
    }
  }
  return simulations >= 50000;
}

