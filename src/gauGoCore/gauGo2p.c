/**
 * @file gauGo2p.c
 * @brief Amazingly simple 2-players go program that uses gauCore library.
 *
 **/

#include "board.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>


/**
 * @brief Extremely simple 2 players go program using libgauCore
 *
 * Usage: gauGo2p [-s boardsize]
 **/
int main(int argc, char** argv)
{
  int boardsize = 9;

  // Parse the command line to scan for options
  int opt = 0;
  while (opt != -1){
    opt = getopt(argc, argv, "s:");

    switch( opt ){
    case 's': boardsize = atoi(optarg); break;
    }
  }

  // Sets up a board
  Board board;
  Board_initialize(&board, boardsize);

  // Game loop
  while(1){
    // Print the board
    Board_print(&board, stdout, 1);

    // Gets move from stdin
    printf("\nmove:");
    char move[8];
    fgets(move, 8, stdin);

    if( strcmp(move, "pass") == 0 ){
      Board_pass(&board);
    }
    else{
      int intersection = Board_intersectionFromName(&board, move);
      if( Board_isLegal(&board, intersection) ){
	Board_play(&board, intersection);
      }
      else{
	printf("Illegal move.\n");
      }
    }
  }
}
