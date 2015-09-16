/**
 * @brief A solver of most sudokus
 *
 * This solver employs very simple techniques to solve sudokus, and doesn't
 * attempt to do anything fancy in order to solve a sudoku. Throughout it uses
 * indexes to identify locations, and the indexes are as:
 *
 *  +---+---+---+
 *  | 0 | 1 | 2 |
 *  +---+---+---+
 *  | 3 | 4 | 5 |
 *  +---+---+---+
 *  | 6 | 7 | 8 |
 *  +---+---+---+
 *
 * This indexing applies to both inside each box and also each box for the
 * entire grid. There are two sets of coordinates used, (row, col) and
 * (square, index) where row/col are relative to the top left corner of the
 * sudoku and the (square, index) are the indices of the square within the
 * entire grid and then the index of the cell within that square. Both of these
 * indices are based on the pattern shown above.
 *
 * Using this indexing scheme, the solver systematically places numbers it knows
 * should be placed with 100% certainty through simple elimination. If any one
 * square of the entire grid can only have on number placed inside of it, that
 * number is placed and then the relevant squares are updated.
 *
 * @author Alex Crichton (acrichto)
 */

#include "semisolver.h"

/*
 * This metadata is used to keep track of what squares need what numbers. It is
 * fairly primitive, but it works.
 *
 * Let the value of needed[i][j][k] be n. If n = 0, then sudoku[i][j] will never
 * be (k + 1). If n = k + 1, then sudoku[i][j] could possibly be k + 1. This way
 * if an entire needed array has only one nonzero entry, then that must be the
 * number which is placed at that location of the grid.
 */
typedef char needed_t[SU_GRID_SIZE][SU_GRID_SIZE][SU_GRID_SIZE];

/* Convert from row/col to a square index */
static int coord_to_square(int row, int col) {
  return col / SU_BOX_SIZE + row / SU_BOX_SIZE * SU_BOX_SIZE;
}

/* Convert from row/col to an index within a square */
static int coord_to_idx(int row, int col) {
  return col % SU_BOX_SIZE + row % SU_BOX_SIZE * SU_BOX_SIZE;
}

/* Convert from a square number and index in that square to a row number */
static int square_to_row(int square, int idx) {
  return idx / SU_BOX_SIZE + square / SU_BOX_SIZE * SU_BOX_SIZE;
}

/* Convert from a square number and index in that square to a column number */
static int square_to_col(int square, int idx) {
  return idx % SU_BOX_SIZE + square % SU_BOX_SIZE * SU_BOX_SIZE;
}

/**
 * @brief Determines whether the number num can be placed at (row, col) in the
 *        sudoku grid
 *
 * It is assumed that (row, col) doesn't already have a number at it and that
 * the location can actually contain the number num.
 */
static int can_place(needed_t needed, int row, int col, int num) {
  int i;
  int square = coord_to_square(row, col);

  int cellneed = 0;    /* other numbers possible in this cell */
  int rowneed = 0;     /* other locations 'num' can be in this row */
  int colneed = 0;     /* other locations 'col' can be in this column */
  int squareneed = 0;  /* other locations 'num' can be in this square */

  /* Figure out the *need counts */
  for (i = 0; i < SU_GRID_SIZE; i++) {
    int square_row = square_to_row(square, i);
    int square_col = square_to_col(square, i);
    if (needed[row][col][i] != 0 && i != num)
      cellneed++;
    if (needed[row][i][num] == num + 1 && i != col)
      rowneed++;
    if (needed[i][col][num] == num + 1 && i != row)
      colneed++;
    if (needed[square_row][square_col][num] == num + 1 && i != coord_to_idx(row, col))
      squareneed++;
  }

  /* If any count is 0, then 'num' can 100% for sure go at (row, col) */
  return cellneed == 0 || rowneed == 0 || colneed == 0 || squareneed == 0;
}

/**
 * @brief Updates a cell int he sudoku to contain a new number
 *
 * It is assumed that the number has already been cleared to go at the specified
 * location, and this will update both 'sudoku' and 'needed' with the correct
 * values to reflect the new number bring placed
 */
static void set_cell(sudoku_t sudoku, needed_t needed, int row, int col, int num) {
  int i;
  int square = coord_to_square(row, col);
  sudoku[row][col] = num + 1;

  for (i = 0; i < SU_GRID_SIZE; i++) {
    needed[row][col][i] = 0; /* Nothing needs to be placed at (row, col) */
    needed[row][i][num] = 0; /* 'num' no longer needed in this row */
    needed[i][col][num] = 0; /* 'num' no longer needed in this column */
    /* 'num' no longer needed in this square */
    needed[square_to_row(square, i)][square_to_col(square, i)][num] = 0;
  }
}

/**
 * @brief Simple elimination of possibilities of numbers
 *
 * Given a relevant (row, col) and a number which could possibly go there, this
 * attempts to eliminate 'num' being a possibility in other squares throughout
 * the grid.
 *
 * For example, if 'num' is needed only within the column of the square its
 * contained in, then it can be eliminated from the rest of the squares in the
 * column. As a grid:
 *    123456789
 *   1.........
 *   2.........
 *   3.........
 *   4.........
 *   5.........
 *   6......4..
 *   7....4....
 *   8.......x2
 *   9.4.......
 *
 * Given the above grid, if a '4' is being placed at the location marked 'x',
 * then the squares (8,{1,2,3}) can all be eliminated of the possibility of a 4
 * because the 4 must be in the bottom 3 rows of the 8th column.
 *
 * Regardless, if 'num' is a possibility at (row, col), this function will
 * analyze the rest of the grid and see if it can eliminate 'num' as a
 * possibility from other entries.
 */
static void eliminate(needed_t needed, int row, int col, int num) {
  int i;
  int colneed = 1, rowneed = 1, squarerowneed = 1, squarecolneed = 1;

  int square = coord_to_square(row, col);
  for (i = 0; i < SU_GRID_SIZE; i++) {
    int square_row = square_to_row(square, i);
    int square_col = square_to_col(square, i);
    if (needed[row][i][num] == num + 1 && coord_to_square(row, i) != square)
      rowneed = 0;
    if (needed[i][col][num] == num + 1 && coord_to_square(i, col) != square)
      colneed = 0;
    if (needed[square_row][square_col][num] == num + 1 && square_row != row)
      squarerowneed = 0;
    if (needed[square_row][square_col][num] == num + 1 && square_col != col)
      squarecolneed = 0;
  }

  if (rowneed) {
    for (i = 0; i < SU_GRID_SIZE; i++)
      if (square_to_row(square, i) != row)
        needed[square_to_row(square, i)][square_to_col(square, i)][num] = 0;
  }
  if (colneed) {
    for (i = 0; i < SU_GRID_SIZE; i++)
      if (square_to_col(square, i) != col)
        needed[square_to_row(square, i)][square_to_col(square, i)][num] = 0;
  }
  if (squarerowneed) {
    for (i = 0; i < SU_GRID_SIZE; i++)
      if (coord_to_square(row, i) != square)
        needed[row][i][num] = 0;
  }
  if (squarecolneed) {
    for (i = 0; i < SU_GRID_SIZE; i++)
      if (coord_to_square(i, col) != square)
        needed[i][col][num] = 0;
  }
}

/**
 * @brief Runs once through a sudoku, trying to solve it
 *
 * This is meant to be one iteration of a loop, and this will return if the
 * sudoku has been fully solved or if there's more work to be done.
 */
static int solve_known(sudoku_t sudoku, needed_t needed) {
  int i, j, k;
  int done = 1;
  for (i = 0; i < SU_GRID_SIZE; i++) {      /* number being placed */
    for (j = 0; j < SU_GRID_SIZE; j++) {    /* row being placed at */
      for (k = 0; k < SU_GRID_SIZE; k++) {  /* column being placed at */
        /* First attempt to eliminate some possibilities in the grid */
        if (needed[j][k][i] == i + 1) {
          eliminate(needed, j, k, i);
        }
        /*
         * If the sudoku is blank at this spot, the number in question is needed
         * here, and we can indeed place the number here, then place it
         */
        if (sudoku[j][k] == 0 && needed[j][k][i] == i + 1 &&
            can_place(needed, j, k, i)) {
          set_cell(sudoku, needed, j, k, i);
        }
        /* If we still have a blank, then there's work left to be done */
        if (sudoku[j][k] == 0)
          done = 0;
      }
    }
  }
  return done;
}

/**
 * @brief Attempts to solve the given sudoku in place.
 *
 * @return 1 if the sudoku has been solved, or 0 if it has not been solved
 */
int semisolve(sudoku_t sudoku) {
  int i, j, k, m;
  needed_t needed;

  /* initialize needed */
  for (i = 0; i < SU_GRID_SIZE; i++) {
    for (j = 0; j < SU_GRID_SIZE; j++) {
      for (k = 0; k < SU_GRID_SIZE; k++) {
        needed[i][j][k] = SU_GRID_SIZE + 1;
      }
    }
  }
  for (i = 0; i < SU_GRID_SIZE; i++) {
    for (j = 0; j < SU_GRID_SIZE; j++) {
      for (k = 0; k < SU_GRID_SIZE; k++) {
        if (sudoku[i][j] == k + 1) {
          int square = coord_to_square(i, j);
          for (m = 0; m < SU_GRID_SIZE; m++) {
            needed[i][j][m] = 0;
            needed[i][m][k] = 0;
            needed[m][j][k] = 0;
            needed[square_to_row(square, m)][square_to_col(square, m)][k] = 0;
          }
        } else if (needed[i][j][k] != 0) {
          needed[i][j][k] = k + 1;
        }
      }
    }
  }

  /* Now attempt to solve the grid */
  for (i = 0; i < SEMI_ITERATIONS; i++) {
    if (solve_known(sudoku, needed))
      return 1;
  }
  return 0;
}
