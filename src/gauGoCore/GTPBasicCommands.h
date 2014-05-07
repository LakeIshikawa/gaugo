/**
 * @file  GTPBasicCommands.h
 * @brief Implementation of basic GTP control commands
 *
 **/
#ifndef GTP_BASIC_COMMANDS_H
#define GTP_BASIC_COMMANDS_H

#include "gauGoEngine.h"

/**
 * @brief Lists available GTP commands
 **/
void GTPBasicCommands_listCommands( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Prints program's name
 **/
void GTPBasicCommands_name( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Prints protocols's version
 **/
void GTPBasicCommands_protocolversion( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Prints program's version
 **/
void GTPBasicCommands_version( GauGoEngine* engine, int argc, char** argv );


/**
 * @brief Play the specified stone on the board.
 * The specified move must be legal and the specified color
 * must be current's turn color.
 *
 **/
void GTPBasicCommands_play( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Generate a move for the current turn.
 * The specified color must be current's turn color. 
 * Move selection is performed by a UCT-MC search as specified in the options.
 **/
void GTPBasicCommands_genmove( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Undos the last move
 **/
void GTPBasicCommands_undo( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Redoes last undone move
 **/
void GTPBasicCommands_redo( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Exists the program
 **/
void GTPBasicCommands_quit( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Print the board for debugging purposes.
 **/
void GTPBasicCommands_printboard( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Adjust board's size and reset
 **/
void GTPBasicCommands_boardsize( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Sets the komi
 **/
void GTPBasicCommands_komi( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Clear the board
 **/
void GTPBasicCommands_clearboard( GauGoEngine* engine, int argc, char** argv );

#endif
