/**
 * @file  gauGoEngine.h
 * @brief Types and definitions of the gauGoEngine
 *
 * The engine is state-based and is controller by GTP commands (see gauGoMain.c),
 * this file defines structure data types to define an engine state to be shared 
 * among all gtp commands.
 *  
 **/
#ifndef GAUGO_ENGINE_H
#define GAUGO_ENGINE_H

#include "board.h"
#include "uctTree.h"
#include "options.h"

#define HISTORY_LENGTH_MAX 512

/**
 * @brief GTP errors
 *
 **/
typedef enum GTPError 
  {
    UNKOWN_COMMAND,
    ILLEGAL_MOVE,
    WRONG_COLOR,
    INVALID_SIZE,
    FILE_NOT_FOUND,
    
    BAD_DATA

  } GTPError;

/**
 * @brief Engine state to be single global state
 * of the GTP program.
 **/
typedef struct GauGoEngine
{
  /** The current go board */
  Board* board;

  /** History */
  Board history[HISTORY_LENGTH_MAX];
  INTERSECTION historyMoves[HISTORY_LENGTH_MAX];
  int historyLength;
  int currentHistoryPos;

  /** Last search tree */
  UCTTree lastTree;

  /** Options */
  Options options;

} GauGoEngine;

/**
 * @brief Initializes the engine with specified program arguments.
 *
 * @param engine The engine
 * @param argc Command-line program arguments count
 * @param argc Command-line program arguments
 **/
int GauGoEngine_initialize( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Reset engine board and history to initial position
 *
 * @param engine The engine
 **/
void GauGoEngine_resetBoard( GauGoEngine* engine );

/**
 * @brief Gets current poisition in tree if available
 *
 * @param engine The engine
 * @return The UCT node corresponding to current position 
 * if any, NULL otherwise
 **/
UCTNode* GauGoEngine_getTreePos( GauGoEngine* engine );

/**
 * @brief Processes a received GTP command
 *
 * @param engine The engine
 * @param argc Command's arguments count
 * @param argc command's arguments
 **/
void GauGoEngine_receiveGTPCommand( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Play a move to the engine's board.
 * This method also saves current position in history, so that the 
 * move can be undone later on
 *
 * @param engine The engine
 * @param move The move
 **/
void GauGoEngine_play(GauGoEngine* engine, INTERSECTION move);

/**
 * @brief Undo last move if possible
 *
 * @param engine The engine
 * @return 1-undo succeeded 0-no moves to undo(now at root)
 **/
int GauGoEngine_undo(GauGoEngine* engine);

/**
 * @brief Redo last undo move if any
 *
 * @param engine The engine
 * @return 1-redo succeeded 0-no moves to redo
 **/
int GauGoEngine_redo(GauGoEngine* engine);

/**
 * @brief Send GTP error response to stdout
 *
 * @param error GTP error to send to stdout
 **/
void GauGoEngine_sayError(GTPError error);

/**
 * @brief Send GTP success response to stdout
 *
 * @param response GTP response to send to stdout
 **/
void GauGoEngine_saySuccess(char* response);

/**
 * @brief Prints the list of all available GTP commands
 * to the specified stream, every entry with the specified format.
 *
 * @param stream Output stream
 * @param format Format string containing exactly ont '%s' that will be replaced
 * with every command name
 **/
void GauGoEngine_printCommandList(FILE* stream, char* format);

#endif
