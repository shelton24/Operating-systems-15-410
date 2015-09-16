/** @file sokoban.h
 *  @brief Level-description interface
 *
 *  @author Michael Berman (mberman)
 */

#ifndef _SOKOBAN_H_
#define _SOKOBAN_H_

#define MAX_LEVELS 32

// a struct containing information about one level in sokoban 
// The actual level map is stored a string in the member variable
// map.

// Here is the "decoder ring" for the strings in the level structs.
//
// You may render these differently on-screen if you wish.

#define SOK_WALL ('X')
#define SOK_PUSH ('o')
#define SOK_ROCK ('b')
#define SOK_GOAL ('g')

typedef struct sokolevel {
  int width;
  int height;
  char * map;
} sokolevel_t;

sokolevel_t *soko_levels[MAX_LEVELS];
int soko_nlevels;

#endif /* _SOKOBAN_H_ */
