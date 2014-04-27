/**
 * @file gauGoEngine.c
 * @brief GauGo GTP engine implementation.
 **/

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "gauGoEngine.h"
#include "GTPBasicCommands.h"
#include "GTPArchiving.h"

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
  { "quit", &GTPBasicCommands_quit },
  { "komi", &GTPBasicCommands_komi },
  { "showboard", &GTPBasicCommands_printboard },
  { "boardsize", &GTPBasicCommands_boardsize },
  { "clear_board", &GTPBasicCommands_clearboard },
  { "name", &GTPBasicCommands_name },
  { "protocol_version", &GTPBasicCommands_protocolversion },
  { "version", &GTPBasicCommands_version },

  // Archiving
  { "load", &GTPArchiving_loadSGF },
  { NULL, NULL }
};


int GauGoEngine_initialize( GauGoEngine* engine, int argc, char** argv )
{
  // Parses command-line options
  Options_initialize( &engine->options, argc, argv );

  // Initializes the board
  Board_initialize( &engine->board, engine->options.boardSize );

  // Randomize
  srand( time(NULL) );

  return 1;
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
