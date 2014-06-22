/**
 * @file  GTPBench.h
 * @brief Implementation of some benchmark GTP commands
 *
 **/
#ifndef GTP_BENCH_H
#define GTP_BENCH_H

#include "gauGoEngine.h"

/**
 * @brief Performs a live benchmark of random playouts
 * without tree search
 **/
void GTPBench_playoutBench( GauGoEngine* engine, int argc, char** argv );

#endif
