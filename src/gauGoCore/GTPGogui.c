/**
 * @file  GTPGogui.c
 * @brief Implementation of some GTP synching commands
 * for use with gogui
 **/

#include <assert.h>
#include <stdlib.h>
#include "GTPGogui.h"
#include "board.h"

// gogui_analyze commands declaration
const char* goguiAnalyzeCommands[] = {
  "gfx/NodeInfo/gogui-nodeinfo",
  NULL
};

void GTPGogui_analyzecommands( GauGoEngine* engine, int argc, char** argv )
{
  printf("= ");

  int i=0;
  while( goguiAnalyzeCommands[i] != NULL ){
    printf("%s", goguiAnalyzeCommands[i]);
    printf("\n");
    i++;
  }

  printf("\n");
  fflush(stdout);
}

int uctNodeCmp(const void* a, const void* b){
  return (*(UCTNode**)b)->played - (*(UCTNode**)a)->played;
}

void GTPGogui_nodeinfo( GauGoEngine* engine, int argc, char** argv )
{
  printf("= ");

  UCTNode* pos = GauGoEngine_getTreePos( engine );
  if( pos ){

    // Briefly sort nodes
    const UCTNode* sortedNodes[MAX_INTERSECTION_NUM];
    int sortedNum=0;
    foreach_child( pos ){
      sortedNodes[sortedNum++] = child;
    }

    // Sort!
    qsort(sortedNodes, sortedNum, sizeof(*sortedNodes), &uctNodeCmp);

    for( int i=0; i<sortedNum; i++ ) {
      char intName[5];
      Board_intersectionName( engine->board, sortedNodes[i]->move, intName );
      printf( "TEXT %s: (wr)%d/%d (amaf)%d/%d (uct)%f\n", 
	      intName, 
	      sortedNodes[i]->winsBlack, 
	      sortedNodes[i]->played,
	      sortedNodes[i]->AMAFwinsBlack,
	      sortedNodes[i]->AMAFplayed,
	      UCTNode_evaluateUCT( sortedNodes[i], pos, engine->board->turn, 0.44f )
	      );
      if( i<5 ){
	printf( "LABEL %s %d\n", intName, i+1 );
      }
    }
  }

  printf("\n\n");
  fflush(stdout);
}
