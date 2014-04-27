/**
 * @file gauGoMain.c
 * @brief GauGo GTP engine implementation.
 * The engine conforms to and extends the GTP protocol to be
 * controller from other programs or directly from the command line.
 *
 * This file controls the main function and program entry point behaviour.
 * 
 **/

#include <stdio.h>
#include <string.h>
#include <wordexp.h>

#include "gauGoEngine.h"

/** @brief Quite standard buffer size */
#define BUF_SIZE 512

/**
 * @brief GauGo GTP engine main
 **/
int main(int argc, char** argv)
{
  // Single global state of the program
  GauGoEngine engine;
  if (!GauGoEngine_initialize( &engine, argc, argv ) ){
    // Initialization error, stop
    return 1;
  }
  
  // Main thread: wait for GTP commands
  char inputBuffer[BUF_SIZE];
  while(1){
    if( fgets( inputBuffer, BUF_SIZE, stdin ) == NULL ){
      // Input closed (parent program terminated?), stop
      return 0;
    }
    fflush(stdin);

    // Remove trailing /n character
    inputBuffer [ strlen(inputBuffer) - 1 ] = '\0';
    
    // Parse the GTP command (POSIX wordexp)
    wordexp_t args;
    int res = wordexp( inputBuffer, &args, 0 );
    if( res == 0 ){
      GauGoEngine_receiveGTPCommand( &engine, args.we_wordc, args.we_wordv );
    }
    else{
      GauGoEngine_sayError(UNKOWN_COMMAND);
    }
    
    // Free wordexp
    wordfree( &args );
  }
}
