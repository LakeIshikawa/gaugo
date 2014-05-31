/**
 * @file  GTPPatterns.c
 * @brief Implementation of some GTP commands
 * for checking 3x3 patterns informations
 **/

#include <assert.h>
#include <stdlib.h>
#include "GTPPatterns.h"
#include "board.h"
#include "uctTree.h"

void GTPPatterns_pattern( GauGoEngine* engine, int argc, char** argv )
{
  if( argc<2 ){
    GauGoEngine_sayErrorCustom("must provide an intersection name");
    return;
  }
  
  INTERSECTION intersection = 
    Board_intersectionFromName( engine->board, argv[1] );
  if( intersection < 0 ){
    GauGoEngine_sayErrorCustom("invalid intersection name");
    return;
  }
  
  int patt = engine->board->patterns3x3[intersection];
  printf("= %x (", patt);

  for( int i=7; i>=0; i-- ){
    printf("%d", (patt&(1<<(i*3+2)))>0);
    printf("%d", (patt&(1<<(i*3+1)))>0);
    printf("%d", (patt&(1<<(i*3)))>0);
    if( i!=0 ) printf("-");
  }

  printf(")\n\n");
  fflush(stdout);
}
