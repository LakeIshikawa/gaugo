/**
 * @file  crash.h
 * @brief Provides crash/assertion failures logging functionality
 *
 * Functions to perform assertions and generate
 * verbose failure log with all sorts of informations available.
 *  
 **/
#ifndef CRASH_H
#define CRASH_H

#include <stdio.h>
#include "global_defs.h"

// Forward refs
struct Board;
struct UCTSearch;

// Debug and release definition of assertion macro
#ifdef NDEBUG
#define gauAssert(...)
#define gauAssertMsg(...)
#define printTrace(...)
#else

/**
 * @brief Asserts that the given expression is true.  If it isn't, 
 * generates a verbose crash log and writes it as "gaucrashN.log" in
 * the current directory.  All information available regarding stack-trace,
 * board state and uct tree will be dumped if possible.
 *
 * @param exp The expression to assert
 * @param board (optional) Current board state
 * @param tree (optional) Current uct tree state
 **/
#define gauAssert(exp, board, tree) _gauAssert(exp, board, tree)

/**
 * @brief Same as gauAssert, but with additional info message to log
 *
 * @param exp The expression to assert
 * @param board (optional) Current board state
 * @param tree (optional) Current uct tree state
 * @param msg Additional info message
 **/
#define gauAssertMsg(exp, board, tree, ...) _gauAssertMsg(exp, board, tree, __VA_ARGS__)

/**
 * @brief Prints current stack trace to specified stream
 *
 * @param stream The target stream to write to
 **/
#define printTrace(stream) _printTrace(stream)

#endif


// Functions wrapped by macros
void _gauAssert(int exp, struct Board* board, struct UCTSearch* tree);
void _gauAssertMsg(int exp, struct Board* board, struct UCTSearch* tree, char* format, ...);
void _printTrace(FILE* stream);

#endif
