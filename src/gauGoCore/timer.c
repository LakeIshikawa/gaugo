/**
 * @file timer.c
 * @brief Timer utility
 *
 **/

#include "timer.h"

void Timer_initialize( Timer* timer )
{
  timer->elapsed = 0;
  timer->running = 0;
}

/**
 * @brief Starts a timer if unstarted, or resume if stopped
 **/
void Timer_start( Timer* timer )
{
  timer->startTime = clock();
  timer->running = 1;
}

/**
 * @brief Stops the timer. The timer can then be resumed.
 **/
void Timer_stop( Timer* timer )
{
  clock_t now = clock();
  timer->elapsed += (double)(now - timer->startTime) / CLOCKS_PER_SEC;
  timer->running = 0;
}

/**
 * @brief Resets a stopped timer. Only works if stopped.
 **/
void Timer_reset( Timer* timer )
{
  if( timer->running == 0 ){
    timer->elapsed = 0;
  }
}

/**
 * @brief Gets current elapsed time
 **/
int Timer_getElapsedTime( Timer* timer )
{
  clock_t now = clock();
  double millis = (double)(now - timer->startTime)*1000 / CLOCKS_PER_SEC;
  return timer->elapsed + millis;
}

