/** @file sokoban.c
 *  @brief Level description information.
 *
 *  @author Michael Berman (mberman)
 */

#include "sokoban.h"

sokolevel_t level1 = {9, 9, "\
   XXX   \
   XgX   \
  XX XX  \
XXX b XXX\
Xg bob gX\
XXX b XXX\
  XX XX  \
   XgX   \
   XXX   "};

sokolevel_t level2 = {9, 9, "\
XXXXX    \
Xo  X    \
X bbX XXX\
X b X XgX\
XXX XXXgX\
 XX    gX\
 X   X  X\
 X   XXXX\
 XXXXX   "};

sokolevel_t level3 = {19, 11, "\
    XXXXX          \
    X   X          \
    Xb  X          \
  XXX  bXX         \
  X  b b X         \
XXX X XX X   XXXXXX\
X   X XX XXXXX  ggX\
X b  b          ggX\
XXXXX XXX XoXX  ggX\
    X     XXXXXXXXX\
    XXXXXXX        "};

sokolevel_t level4 = {10, 7, "\
 XXXXXXX  \
 X     XXX\
XXbXXX   X\
X o b  b X\
X ggX b XX\
X ggX   X \
XXXXXXXXX "};

sokolevel_t level5 = {14, 10, "\
XXXXXXXXXXXX  \
Xgg  X     XXX\
Xgg  X b  b  X\
Xgg  XbXXXX  X\
Xgg    o XX  X\
Xgg  X X  b XX\
XXXXXX XXb b X\
  X b  b b b X\
  X    X     X\
  XXXXXXXXXXXX"};

sokolevel_t level6 = {32, 15, "\
 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX \
XX                            XX\
X           bbb   bbbb         X\
X          b   b b             X\
X   o      b   b  bbb          X\
X          b   b     b         X\
X           bbb  bbbb          X\
X                              X\
X XXXX   ggg   XXXX X  X  gggg X\
X X  XX g   g XX    X XX g     X\
X XXXX  g   g X     XXX   ggg  X\
X X  XX g   g XX    X XX     g X\
X X   X  ggg   XXXX X  X gggg  X\
X                              X\
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"};

sokolevel_t * soko_levels[] = {&level1, &level2, &level3, &level4, &level5, &level6, 0};
int soko_nlevels = 6; // The array may have some spare slots... 
