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
#include "options.h"

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

  } GTPError;

/**
 * @brief Engine state to be single global state
 * of the GTP program.
 **/
typedef struct GauGoEngine
{
  /** The go board */
  Board board;

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
 * @brief Processes a received GTP command
 *
 * @param engine The engine
 * @param argc Command's arguments count
 * @param argc command's arguments
 **/
void GauGoEngine_receiveGTPCommand( GauGoEngine* engine, int argc, char** argv );

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
