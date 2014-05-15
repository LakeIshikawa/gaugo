/**
 * @file timer.h
 * @brief Timer utility
 *
 **/

#include <time.h>
#include <stdio.h>

/**
 * A very simple timer
 **/
typedef struct Timer
{
  // Time of last time the timer got started (or resumed)
  clock_t startTime;

  // Total elapsed time
  int elapsed;

  // Running state
  int running;

} Timer;

/**
 * @brief Initializes timer to default state
 **/
void Timer_initialize( Timer* timer );

/**
 * @brief Starts a timer if unstarted, or resume if stopped
 **/
void Timer_start( Timer* timer );

/**
 * @brief Stops the timer. The timer can then be resumed.
 **/
void Timer_stop( Timer* timer );

/**
 * @brief Resets a stopped timer. Only works if stopped.
 **/
void Timer_reset( Timer* timer );

/**
 * @brief Gets current elapsed time
 **/
int Timer_getElapsedTime( Timer* timer );
