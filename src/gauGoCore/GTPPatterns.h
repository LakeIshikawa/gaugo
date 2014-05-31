/**
 * @file  GTPPatterns.h
 * @brief Implementation of some GTP commands
 * for checking 3x3 patterns informations
 *
 **/
#ifndef GTP_PATTERNS_COMMANDS_H
#define GTP_PATTERNS_COMMANDS_H

#include "gauGoEngine.h"

/**
 * @brief Outputs current pattern bit representation
 * of given intersection
 **/
void GTPPatterns_pattern( GauGoEngine* engine, int argc, char** argv );

#endif
