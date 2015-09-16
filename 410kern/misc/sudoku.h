/**
 * @file sudoku.h
 * @brief Definitions for sudoku sizes and sudoku types
 *
 * @author Alex Crichton (acrichto)
 */

#ifndef _SUDOKU_H
#define _SUDOKU_H

/* Size of each "box" in the sudoku grid */
#define SU_BOX_SIZE 3

/* Size of the actual grid (which is composed of boxes */
#define SU_GRID_SIZE (SU_BOX_SIZE * SU_BOX_SIZE)

/* Total number of cells for a number in a grid */
#define SU_GRID_AREA (SU_GRID_SIZE * SU_GRID_SIZE)

/*
 * Each sudoku number is stored as the integer number, not the character. For
 * example the number 2 would be stored as 0x02, not '2' = 0x32.
 *
 * A blank cell is marked as a 0x00, not a ' ' = 0x20
 */
typedef char sudoku_t[SU_GRID_SIZE][SU_GRID_SIZE];

#endif /* _SUDOKU_H */
