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
#include "p3x3info.h"

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
  printf("= %x (%d|%d|%d|%d)(", 
	 patt,
	 (patt>>19)&1,
	 (patt>>18)&1,
	 (patt>>17)&1,
	 (patt>>16)&1
	 );

  for( int i=7; i>=0; i-- ){
    printf("%d", (patt&(1<<(i*2+1)))>0);
    printf("%d", (patt&(1<<(i*2)))>0);
    if( i!=0 ) printf("-");
  }

  printf(")\n\n");
  fflush(stdout);
}

void GTPPatterns_patternInfo( GauGoEngine* engine, int argc, char** argv )
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
  printf("= %x (%d-%d-%d)\n\n", 
	 p3x3info[patt],
	 p3x3info[patt]>>2,
	 (p3x3info[patt]&2)>0,
	 (p3x3info[patt]&1)>0
	 );
  fflush(stdout);
}
