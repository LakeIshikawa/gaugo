/**
 * @file  GTPSynching.h
 * @brief Implementation of some GTP synching commands
 *
 **/
#ifndef GTP_SYNCHING_COMMANDS_H
#define GTP_SYNCHING_COMMANDS_H

#include "gauGoEngine.h"

/**
 * @brief Outputs board informations that are relevant for a UI
 **/
void GTPSynching_board( GauGoEngine* engine, int argc, char** argv );

/**
 * @brief Outputs ucttree nodes information about current position's
 * childrens, if any ucttree is available (i.e. if a previous genmove 
 * exists)
 **/
void GTPSynching_ucttree( GauGoEngine* engine, int argc, char** argv );

#endif
