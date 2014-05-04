/**
 * @file  GTPArchiving.h
 * @brief Implementation of some GTP archiving commands
 *
 **/
#ifndef GTP_ARCHIVING_COMMANDS_H
#define GTP_ARCHIVING_COMMANDS_H

#include "gauGoEngine.h"

/**
 * @brief Load the main variation of an sgf file
 **/
void GTPArchiving_loadSGF( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Save current board position in sgf format (single variation)
 **/
void GTPArchiving_saveSGF( GauGoEngine* engine, int argc, char** argv );

#endif
