/**
 * @file nonogram_db.c
 * @brief A "database" of Nonogram board layouts.
 *
 * @author Dave Eckhardt (de0u)
 */

#include "nonogram_db.h"

#define END  NG_RUN_END
#define LAST NG_RUN_LAST

ng_layout_t ng_layouts[] = {

  // A "tutorial" layout
  { 1, 5,
    { 2, 2, END, LAST },
    { 1, END, 1, END, END, 1, END, 1, END, LAST }
  },

  // malehorn: silly #1
  { 10, 10,
    { 1, END, 1, END, 1, END, 1, END, 1, END,
      1, END, 1, END, 1, END, 1, END, 1, END, LAST },
    { 1, END, 1, END, 1, END, 1, END, 1, END,
      1, END, 1, END, 1, END, 1, END, 1, END, LAST }
  },

  // A layout observed on http://www.goobix.com/games/nonograms
  { 5, 5,
    { 2, 2, END, 1, END, 2, END, 2, END, 3, END, LAST },
    { 1, END, 1, 1, END, 3, END, 5, END, 1, END, LAST }
  },

  // malehorn: "random 5x5"
  { 5, 5,
      { 1, END, 4, END, 2, 2, END, 1, 3, END, 3, END, LAST },
      { 3, END, 2, END, 2, 2, END, 4, END, 3, END, LAST }
  },

  // de0u
  { 5, 9,
    { 1, 1, 1, 3, END,
      1, 1, 1, 1, 1, END,
      3, 1, 1, 1, END,
      1, 1, 1, 1, END,
      1, 1, 3, END, LAST },
    { 3, END,
      1, END,
      5, END,
      END,
      5, END,
      END,
      5, END,
      1, 1, END,
      5, END, LAST }
  }

};

int ng_layout_count = sizeof(ng_layouts) / sizeof(ng_layouts[0]);
