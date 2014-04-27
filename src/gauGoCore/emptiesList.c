/**
 * @file emptiesList.c
 * @brief Empties list implementation
 *
 **/

#include "emptiesList.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define link(a, b) a->next = b; b->previous = a;

void EmptiesList_initialize( EmptiesList* list, BoardIterator* boardIter, Board* board )
{
  // Inits list
  list->length = 0;
  list->firstEmpty = NULL;
  list->lastEmpty = NULL;

  foreach_intersection( boardIter ){
    list->pool[intersection].intersection = intersection;
    list->pool[intersection].previous = NULL;
    list->pool[intersection].next = NULL;
    
    if( Board_getColor( board, intersection ) == EMPTY ){
      // Adds the empty
      EmptiesList_add( list, intersection );
    }
  }
}

void EmptiesList_add( EmptiesList* list, INTERSECTION intersection )
{
  if( list->firstEmpty == NULL ){
    list->firstEmpty = &list->pool[intersection];
    list->firstEmpty->next = NULL;
    list->firstEmpty->previous = NULL;
    list->lastEmpty = list->firstEmpty;
  }
  else{
    EmptyNode* new = &list->pool[intersection];
    new->next = NULL;

    link(list->lastEmpty, new);

    list->lastEmpty = new;
  }

  list->length++;
}

void EmptiesList_delete( EmptiesList* list, INTERSECTION intersection )
{
  EmptyNode* remove = &list->pool[intersection];
  if( remove == list->firstEmpty ){
    list->firstEmpty = remove->next;
    list->firstEmpty->previous = NULL;
    
    remove->next = NULL;
  }
  else if( remove == list->lastEmpty ){
    list->lastEmpty = remove->previous;
    list->lastEmpty->next = NULL;

    remove->previous = NULL;
  }

  else{
    link(remove->previous, remove->next);
    
    remove->previous = NULL;
    remove->previous = NULL;
  }

  list->length--;
}

void EmptiesList_print( EmptiesList* list )
{
  printf("el=[");
  foreach_empty( list ){
    printf("%d ", emptyNode->intersection);
  }
  printf("]\n");
}
