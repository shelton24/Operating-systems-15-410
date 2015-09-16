/**
 * @file nonogram.h
 * @brief Definitions for Nonogram sizes types
 *
 * @author Dave Eckhardt (de0u)
 */

#ifndef _NONOGRAM_DB_H
#define _NONOGRAM_DB_H

/** Maximum number of rows in a layout */
#define NG_MAX_ROWS 10

/** Maximum number of columns in a layout */
#define NG_MAX_COLS 10

/** Maximum number of elements/entries in a "run array".
 *  The number given is a crudely computed over-estimate
 *  of the true value.
 */
#define NG_RUN_ELTS (NG_MAX_ROWS * NG_MAX_COLS)

/*! \def NG_RUN_END
 *  \brief
 *  Special value in a run list that indicates "end of row" (or column)
 */
#define NG_RUN_END (0)

/*! \def NG_RUN_LAST
 *  \brief
 *  Special value in a run list that indicates "no more rows" (or columns)
 */
#define NG_RUN_LAST (-1)

typedef struct ng_layout {
  /** Rows for the board that uses this layout */
  int rows;

  /** Cols for the board that uses this layout */
  int cols;

  /**
   * "Row runs": for example "2, 3, END" means "two runs, one of
   * length 2, one of length 3, then nothing more for that row".
   * Each array will end with "END, LAST", where LAST means "no
   * more rows follow".
   */
  int row_runs[NG_RUN_ELTS];

  /**
   * "Col runs": for example "2, 3, END" means "two runs, one of
   * length 2, one of length 3, then nothing more for that column".
   * Each array will end with "END, LAST", where LAST means "no
   * more columns follow".
   */
  int col_runs[NG_RUN_ELTS];
} ng_layout_t;

extern ng_layout_t ng_layouts[];
extern int ng_layout_count;

#endif /* _NONOGRAM_DB_H */
