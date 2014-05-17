/**
 * @file  GTPBasicCommands.c
 * @brief Implementation of basic GTP control commands
 *
 **/

#include <string.h>
#include <stdlib.h>

#include "GTPBasicCommands.h"
#include "uctSearch.h"
#include "policies.h"
#include "stoppers.h"
#include "hashTable.h"
#include "crash.h"

void GTPBasicCommands_listCommands( GauGoEngine* engine, int argc, char** argv )
{
  printf("= ");
  GauGoEngine_printCommandList(stdout, "%s\n");
  printf("\n");
  fflush(stdout);
}

void GTPBasicCommands_name( GauGoEngine* engine, int argc, char** argv )
{
  GauGoEngine_saySuccess("GauGo");  
}

void GTPBasicCommands_protocolversion( GauGoEngine* engine, int argc, char** argv )
{
  GauGoEngine_saySuccess("2");
}

void GTPBasicCommands_version( GauGoEngine* engine, int argc, char** argv )
{
  GauGoEngine_saySuccess("0.1");
}


void GTPBasicCommands_play( GauGoEngine* engine, int argc, char** argv )
{
  // Must be 3-args command included
  if( argc != 3 ) {
    GauGoEngine_sayError( UNKOWN_COMMAND );
    return;
  }

  // Pass once if wrong color
  if( ! ( ((strcmp( argv[1], "w" ) == 0 || (strcmp( argv[1], "W" ) == 0)) 
	   && engine->board->turn == WHITE)
	  || ((strcmp( argv[1], "b" ) == 0 || (strcmp( argv[1], "B" ) == 0)) 
	      && engine->board->turn == BLACK) ) ) {
    GauGoEngine_play(engine, PASS);
  }

  // Check for the pass move
  if( strcmp( argv[2], "pass" ) == 0 || strcmp( argv[2], "PASS" ) == 0 ){
    // Pass
    GauGoEngine_play(engine, PASS);
    GauGoEngine_saySuccess("");
    return;
  }

  // Check move legality
  int arglen = strlen( argv[2] );
  if( arglen < 2 || arglen > 3){
    GauGoEngine_sayError( ILLEGAL_MOVE );
    return;
  }

  INTERSECTION move = Board_intersectionFromName( engine->board, argv[2] );
  if( move == -1 || !Board_isLegal( engine->board, move ) ){
    GauGoEngine_sayError( ILLEGAL_MOVE );
    return;
  }

  // Play the move
  GauGoEngine_play(engine, move);
  GauGoEngine_saySuccess("");
}

void GTPBasicCommands_genmove( GauGoEngine* engine, int argc, char** argv )
{
  // Must be 2-args command included
  if( argc != 2 ) {
    GauGoEngine_sayError( UNKOWN_COMMAND );
    return;
  }

  // If no legal moves to play, play pass without thinking
  BoardIterator it;
  Board_iterator( engine->board, &it );
  if( Board_mustPass( engine->board, &it ) ){
    GauGoEngine_play(engine, PASS);
    GauGoEngine_saySuccess("pass");
    return;
  }

  // Initializes new empty tree
  UCTTree_delete( &engine->lastTree );
  UCTTree_initialize( 
		     &engine->lastTree, 
		     engine->options.treePoolNodeNum,
		     engine->board);

  // Last boards' hash keys
  HashKey lastBoards[SUPERKO_HISTORY_MAX];
  int b=0;
  for( int i=engine->currentHistoryPos; i>=0 && b<SUPERKO_HISTORY_MAX; i-- ){
    lastBoards[SUPERKO_HISTORY_MAX-(++b)] = engine->history[i].hashKey;
  }

  // UCT search
  UCTSearch search;
  UCTSearch_initialize( &search, engine->board, 
			&engine->lastTree, &POLICY_pureRandom, 
			&STOPPER_5ksim, &engine->options, lastBoards );
  INTERSECTION move = UCTSearch_search( &search );

  char moveStr[5] = { '\0' };
  if( move == PASS ){
    GauGoEngine_play(engine, PASS);
    strcpy(moveStr, "pass");
  }
  else{
    gauAssert( Board_isLegal( engine->board, move ), engine->board, NULL );
    
    // Play move
    GauGoEngine_play(engine, move);
    Board_intersectionName( engine->board, move, moveStr );
  }
  
  GauGoEngine_saySuccess(moveStr);
}

void GTPBasicCommands_undo( GauGoEngine* engine, int argc, char** argv )
{
  GauGoEngine_undo( engine );
  GauGoEngine_saySuccess("");
}

void GTPBasicCommands_redo( GauGoEngine* engine, int argc, char** argv )
{
  GauGoEngine_redo( engine );
  GauGoEngine_saySuccess("");
}

void GTPBasicCommands_quit( GauGoEngine* engine, int argc, char** argv )
{
  GauGoEngine_saySuccess("");
  exit(0);
}

void GTPBasicCommands_printboard( GauGoEngine* engine, int argc, char** argv )
{
  printf("= ");
  Board_print( engine->board, stdout, 0 );
  printf("\n");
  fflush(stdout);
}

void GTPBasicCommands_boardsize( GauGoEngine* engine, int argc, char** argv )
{
  if( argc != 2 ){
    GauGoEngine_sayError( UNKOWN_COMMAND );
    return;
  }

  int size = atoi( argv[1] );
  if( size < 4 || size > 19 ){
    GauGoEngine_sayError( INVALID_SIZE );
    return;
  }
  
  engine->options.boardSize = size;
  GauGoEngine_resetBoard( engine );
  GauGoEngine_saySuccess("");
}

void GTPBasicCommands_komi( GauGoEngine* engine, int argc, char** argv )
{
  // Must be 2-args command included
  if( argc != 2 ) {
    GauGoEngine_sayError( UNKOWN_COMMAND );
    return;
  }

  engine->options.komi = atof(argv[1]);
  GauGoEngine_saySuccess("");
}

void GTPBasicCommands_clearboard( GauGoEngine* engine, int argc, char** argv )
{
  // Delete tree
  UCTTree_delete( &engine->lastTree );
  UCTTree_initializeEmpty( &engine->lastTree );

  // Reset board
  GauGoEngine_resetBoard( engine );
  GauGoEngine_saySuccess("");
}

void GTPBasicCommands_finalscore( GauGoEngine* engine, int argc, char** argv )
{
  BoardIterator it;
  Board_iterator( engine->board, &it );
  int score = Board_trompTaylorScore( engine->board, &it );

  float finalScore = score - engine->options.komi;
  char scoreBuf[8];
  if( finalScore < 0 ){
    sprintf(scoreBuf, "W+%2.1f", -finalScore);
  } else if( finalScore > 0 ){
    sprintf(scoreBuf, "B+%2.1f", finalScore);
  } else {
    sprintf(scoreBuf, "0");
  }
  
  GauGoEngine_saySuccess(scoreBuf);
}
