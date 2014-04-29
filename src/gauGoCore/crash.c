/**
 * @file crash.c
 * @brief Crash implementation
 *
 **/

#include "crash.h"
#include "board.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>


// Forward declaration
void _gauAssertPvt(int exp, struct Board* board, struct UCTSearch* tree, 
		   char* format, va_list args);

/**
 * @brief Opens the first file named like gaucrashN.log that does
 * not exist and that can be opened in write mode.
 * If no such file exists, returns NULL.
 *
 * @return A file to write log to if possible, NULL otherwise
 **/
FILE* gauOpenLogFile()
{
  // Search for the first non-existant log file
  int i=1;
  char filename[32];
  while(1){
    // Build filename
    sprintf(filename, "gaucrash%d.log", i);

    // Checks file existance
    struct stat buffer;
    if( stat(filename, &buffer) != 0 ) break;

    i++;
  }

  return fopen(filename, "w");
}

void _gauAssert(int exp, struct Board* board, struct UCTSearch* tree)
{
  _gauAssertPvt(exp, board, tree, NULL, NULL);
}

void _gauAssertMsg(int exp, struct Board* board, struct UCTSearch* tree, 
		   char* format, ...)
{
  va_list args;
  va_start(args, format);
  _gauAssertPvt( exp, board, tree, format, args);
  va_end(args);
}

void _gauAssertPvt(int exp, struct Board* board, struct UCTSearch* tree, 
		   char* format, va_list args)
{
  // If assertion is true, don't log anything
  if( exp ) return;

  // Open log file
  FILE* logFile = gauOpenLogFile();
  if( logFile == NULL ){
    fprintf(stderr, "Cannot generate crash log. Try to run gauGo from a different directory\n");
    exit(1);
  }
  
  // Print backtrace
  fprintf(logFile, "------ STACK TRACE -----\n");
  printTrace(logFile);
  fprintf(logFile, "------\\STACK TRACE -----\n\n");

  // Print the board if available
  if( board != NULL ){
    fprintf(logFile, "------ BOARD INFO -----\n");
    Board_print(board, logFile, 1, 1);
    fprintf(logFile, "------\\BOARD INFO -----\n\n");
  }

  // Print additional message
  fprintf(logFile, "------ MESSAGE -----\n");
  vfprintf(logFile, format, args);
  fprintf(logFile, "------\\MESSAGE  -----\n\n");

  // Close the log file
  fclose(logFile);
  
  // Fail the assertion
  assert(0);
}

void _printTrace(FILE* stream)
{
  void *array[20];
  size_t size;
  char **strings;
  size_t i;
  
  size = backtrace (array, 20);
  strings = backtrace_symbols (array, size);
     
  fprintf (stream, "Obtained %zd stack frames.\n", size);
     
  for (i = 0; i < size; i++)
    fprintf (stream, "%s\n", strings[i]);
  
  free (strings);
}
