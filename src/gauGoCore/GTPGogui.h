/**
 * @file  GTPGogui.h
 * @brief Implementation of some GTP synching commands
 * for use with gogui
 *
 **/
#ifndef GTP_GOGUI_COMMANDS_H
#define GTP_GOGUI_COMMANDS_H

#include "gauGoEngine.h"

/**
 * @brief Outputs all available gogui_analyze available commands
 * for use with gogui
 **/
void GTPGogui_analyzecommands( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Outputs pv from current position in gogui gfx format
 * to visualize pv on the board.
 **/
void GTPGogui_pv( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Outputs ucttree nodes information about current position's
 * childrens, if any ucttree is available (i.e. if a previous genmove 
 * exists)
 **/
void GTPGogui_nodeinfo( GauGoEngine* engine, int argc, char** argv );

#endif
