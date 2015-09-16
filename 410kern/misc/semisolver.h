/**
 * @file semisolver.h
 * @brief Headers for almost solving sudokus
 *
 * @author Alex Crichton (acrichto)
 */

#ifndef _SEMISOLVER_H
#define _SEMISOLVER_H

#include "sudoku.h"

/* Number of iterations the semisolver will attempt to solve a grid */
#define SEMI_ITERATIONS 100

int semisolve(sudoku_t sudoku);

#endif /* _SEMISOLVER_H */
