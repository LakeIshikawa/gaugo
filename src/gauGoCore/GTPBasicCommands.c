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
  GauGoEngine_saySuccess("1.0");
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

  // Checek color
  if( ! ( ((strcmp( argv[1], "w" ) == 0 || (strcmp( argv[1], "W" ) == 0)) 
	   && engine->board.turn == WHITE)
	  || ((strcmp( argv[1], "b" ) == 0 || (strcmp( argv[1], "B" ) == 0)) 
	      && engine->board.turn == BLACK) ) ) {
    GauGoEngine_sayError( WRONG_COLOR );
    return;
  }

  // Check for the pass move
  if( strcmp( argv[2], "pass" ) == 0 || strcmp( argv[2], "PASS" ) == 0 ){
    Board_pass( &engine->board );
    GauGoEngine_saySuccess("");
    return;
  }

  // Check move legality
  if( strlen( argv[2] ) != 2 ){
    GauGoEngine_sayError( ILLEGAL_MOVE );
    return;
  }

  INTERSECTION move = Board_intersectionFromName( &engine->board, argv[2] );
  if( move == -1 || !Board_isLegal( &engine->board, move ) ){
    GauGoEngine_sayError( ILLEGAL_MOVE );
    return;
  }

  // Play move
  Board_play( &engine->board, move );
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
  Board_iterator( &engine->board, &it );
  if( Board_mustPass( &engine->board, &it ) ){
    Board_pass( &engine->board );
    GauGoEngine_saySuccess("pass");
    return;
  }

  // Create hash table
  HashTable hashTable;
  HashTable_initialize( &hashTable, engine->options.hashTableSize, sizeof( UCTNode ) );

  // UCT search
  UCTSearch search;
  UCTSearch_initialize( &search, &engine->board, &hashTable, &POLICY_pureRandom, &STOPPER_5ksim, &engine->options );
  INTERSECTION move = UCTSearch_search( &search );

  // Print pv (comment)
#ifdef DEBUG
  UCTSearch_printPv( &search );
#endif

  char moveStr[5] = { '\0' };
  if( move == PASS ){
    Board_pass( &engine->board );
    strcpy(moveStr, "pass");
  }
  else{
    gauAssert( Board_isLegal( &engine->board, move ), &engine->board, NULL );
    
    // Play move
    Board_play( &engine->board, move );
    Board_intersectionName( &engine->board, move, moveStr );
  }
  
  HashTable_delete( &hashTable );
  GauGoEngine_saySuccess(moveStr);
}

void GTPBasicCommands_undo( GauGoEngine* engine, int argc, char** argv )
{
  // TODO
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
  Board_print( &engine->board, stdout, 0, 0 );
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
  Board_initialize( &engine->board, engine->options.boardSize );
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
  Board_initialize( &engine->board, engine->options.boardSize );
  GauGoEngine_saySuccess("");
}
