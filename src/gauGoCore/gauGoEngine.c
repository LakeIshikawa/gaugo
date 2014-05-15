/**
 * @file gauGoEngine.c
 * @brief GauGo GTP engine implementation.
 **/

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "gauGoEngine.h"
#include "GTPBasicCommands.h"
#include "GTPArchiving.h"
#include "GTPSynching.h"

/**
 * @brief GTP command processor function type
 **/
typedef void (*GTPCommandProcessor)(GauGoEngine* engine, int argc, char** argv);

/**
 * @brief Command/Processor pair
 **/
typedef struct CmdAndProcessor { 
  char* commandName;
  GTPCommandProcessor processor;
} CmdAndProcessor;

/**
 * @brief Command name to processor function table
 **/
CmdAndProcessor commandProcessors[] = {
  // Basic
  { "list_commands", &GTPBasicCommands_listCommands },
  { "play", &GTPBasicCommands_play },
  { "genmove", &GTPBasicCommands_genmove },
  { "undo", &GTPBasicCommands_undo },
  { "redo", &GTPBasicCommands_redo },
  { "quit", &GTPBasicCommands_quit },
  { "komi", &GTPBasicCommands_komi },
  { "showboard", &GTPBasicCommands_printboard },
  { "boardsize", &GTPBasicCommands_boardsize },
  { "clear_board", &GTPBasicCommands_clearboard },
  { "name", &GTPBasicCommands_name },
  { "protocol_version", &GTPBasicCommands_protocolversion },
  { "version", &GTPBasicCommands_version },
  { "final_score", &GTPBasicCommands_finalscore },

  // Archiving
  { "load", &GTPArchiving_loadSGF },
  { "save", &GTPArchiving_saveSGF },

  // Synching
  { "board", &GTPSynching_board },
  { "ucttree", &GTPSynching_ucttree },

  { NULL, NULL }
};


int GauGoEngine_initialize( GauGoEngine* engine, int argc, char** argv )
{
  // Parses command-line options
  Options_initialize( &engine->options, argc, argv );
  // Set empty tree
  UCTTree_initializeEmpty(&engine->lastTree);
  // Init board
  GauGoEngine_resetBoard( engine );

  // Randomize
  srand( time(NULL) );

  return 1;
}

void GauGoEngine_resetBoard( GauGoEngine* engine )
{
  // Empty history
  engine->historyLength = 1;
  engine->currentHistoryPos = 0;
  engine->board = &engine->history[0];

  // Initializes the board
  Board_initialize( engine->board, engine->options.boardSize );

  // Init tree to empty
  UCTTree_delete(&engine->lastTree);
  UCTTree_initializeEmpty(&engine->lastTree);
}

UCTNode* GauGoEngine_getTreePos( GauGoEngine* engine )
{
  // If no tree, no position
  if( engine->lastTree.root.firstChild == NULL ) return NULL;

  // Search for root in history
  UCTNode* position = &engine->lastTree.root;
  int rootFound = 0;
  int i;
  for( i=0; i<engine->currentHistoryPos+1; i++ ){
    
    // Remember root when found
    if( engine->history[i].hashKey == engine->lastTree.rootHash ) {
      rootFound = 1;
    }

    // Descend if root present
    else if( rootFound ){
      foreach_child(position){
	if( child->move == engine->historyMoves[i-1] ){
	  position = child;
	  break;
	}
      }
    }
  }

  // No root no pos
  if( !rootFound ) return NULL;

  return position;
}

void GauGoEngine_play(GauGoEngine* engine, INTERSECTION move)
{
  assert( engine->historyLength <= HISTORY_LENGTH_MAX );

  // If the redo is possible
  int redone = 0;
  if( engine->currentHistoryPos < engine->historyLength-1 ) {
    
    // If the move is the redo move, redo it
    if( move == engine->historyMoves[engine->currentHistoryPos] ) {
      // Redo
      redone = 1;
      GauGoEngine_redo(engine);
    }
    else{
      // Destroy all redo positions, and start a new branch
      engine->historyLength = engine->currentHistoryPos+1;
    }
  }

  if( !redone ){
    // Copy current position
    engine->history[ engine->historyLength++ ] = *engine->board;
    engine->historyMoves[ engine->currentHistoryPos++ ] = move;
    engine->board = &engine->history[ engine->currentHistoryPos ];
    
    // Play the move
    if( move == PASS ) Board_pass( engine->board );
    else Board_play( engine->board, move );
  }
}

int GauGoEngine_undo(GauGoEngine* engine)
{
  if( engine->currentHistoryPos == 0 ) return 0;

  // Undo board
  engine->board = &engine->history[--engine->currentHistoryPos];

  return 1;
}

int GauGoEngine_redo(GauGoEngine* engine)
{
  // Redo board
  if( engine->currentHistoryPos < engine->historyLength-1 ) {
    engine->board = &engine->history[++engine->currentHistoryPos];
    return 1;
  }

  return 0;
}

void GauGoEngine_receiveGTPCommand( GauGoEngine* engine, int argc, char** argv )
{
  // No empty commands
  if( argc == 0 ){
    GauGoEngine_sayError(UNKOWN_COMMAND);
    return;
  }

  // Match command
  CmdAndProcessor* it;
  for( it = commandProcessors; it->commandName; it++ ){
    if( strcmp( argv[0], it->commandName ) == 0 ){
      (*(it->processor))(engine, argc, argv);
      return;
    }
  }

  // No match: error
  GauGoEngine_sayError( UNKOWN_COMMAND );
}

/**
 * @brief Send GTP error response to stdout
 *
 * @param error GTP error to send to stdout
 **/
void GauGoEngine_sayError(GTPError error)
{
  switch( error ){
  case UNKOWN_COMMAND: printf("? unkown command\n\n"); break;
  case ILLEGAL_MOVE:   printf("? illegal move\n\n"); break;
  case WRONG_COLOR:  printf("? wrong color\n\n"); break;
  case INVALID_SIZE:  printf("? invalid size\n\n"); break;
  case FILE_NOT_FOUND:  printf("? file not found\n\n"); break;
  case BAD_DATA: printf("? bad data\n\n"); break;
  }

  fflush(stdout);
}

/**
 * @brief Send GTP success response to stdout
 *
 * @param response GTP response to send to stdout
 **/
void GauGoEngine_saySuccess(char* response)
{
  printf("= %s\n\n", response);
  fflush(stdout);
}

void GauGoEngine_printCommandList(FILE* stream, char* format)
{
  CmdAndProcessor* it;
  for( it = commandProcessors; it->commandName; it++ ){
    fprintf(stream, format, it->commandName);
  }
}
