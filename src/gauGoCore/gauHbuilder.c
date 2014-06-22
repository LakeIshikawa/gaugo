/**
 * @file gauHbuilder.c
 * @brief Utility for generating constant header
 * files that are used in gauGo build process.
 *
 **/

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include "string.h"

#include "build3x3info.h"

// Possible command sets
typedef enum Command
  {
    BUILD
  }Command;

// Build some data
int build(char* command, char* outfile);

// Main program
int main( int argc, char **argv ){
  
  // Parse command line options
  static struct option long_options[] = {
    {"build", required_argument, 0,  'b'},
    {0,         0,                 0,  0 }
  };
  int c, option_index;
  char build_command[512];
  char outfile[512];
  Command command;

  // Defaults
  sprintf(outfile, "out.h");

  while(1){
    c = getopt_long(argc, argv, "b:o:", long_options, &option_index);
    if( c==-1 ) break;
    switch(c){
      // Output filename
    case 'o': strcpy(outfile, optarg); break;
      // build
    case 'b' : {
      command = BUILD;
      strcpy(build_command, optarg); 
      break;
    }
    }
  }

  // Process command
  switch( command ) {
  case BUILD: build(build_command, outfile); return 0;
  }
  
  printf("usage : %s --build command\n", argv[0]);
  printf("commands: 3x3info\n");

  return 1;
}


int build(char* command, char* outfile)
{
  // 3x3 info
  if( strcmp(command, "3x3info") == 0 ){
    if( build3x3info(outfile) ){
      return 0;
    }
  }

  return 1;
}
