/**
 * @file sudokudb.h
 * @brief Definitions for a pre-built "database" of sudokus
 *
 * @author Alex Crichton (acrichto)
 */

#ifndef _SUDOKUDB_H
#define _SUDOKUDB_H

/* Difficulty for each sudoku will range from 0 to (this - 1) */
#define MAX_DIFFICULTY 10

/* Maximum number of sudokus contained in the database */
#define MAX_SUDOKUS 100

/*
 * The size of this string is a multiple of (1 + (SU_GRID_SIZE * SU_GRID_SIZE))
 * where the first character is the difficulty, and then the next
 * (SU_GRID_SIZE * SU_GRID_SIZE) characters are the actual sudoku. The first row
 * is listed first, then the second, etc.
 *
 * Everything is stored as character literals. If the difficulty is 4, then the
 * difficulty will be read as '4' = 0x34, not 0x04. A blank cell for each sudoku
 * will be read as a space character (0x20).
 *
 * The end of the sudoku database will be indicated by a 0 byte (the end of the
 * string).
 */
extern const char *sudokudb;

#endif /* _SUDOKUDB_H */
