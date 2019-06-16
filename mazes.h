/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* ways to put a maze in a grid */

#ifndef _MAZES_H
#define _MAZES_H

#include "grid.h"
#include "distance.h"

#define UNVISITED	1
#define VISITED		2
#define WALK_CONSIDER   3
#define NEEDDIR		(99+DIRECTIONS)

/* for sidewinder */
typedef struct {
  int runlength;
  int runstart_r;
  int runstart_c;
} sw_tree_status;

/* iterategrid() call backs; these can generate a "maze" by visiting
 * every cell once in any order.
 */
int btreewalker(GRID *, CELL *, void *);
int sidewinderwalker(GRID *, CELL *, void *);
int serpentine(GRID *, CELL *, void *);
int hollow(GRID *, CELL *, void *);

/* non-iterator generators; these need more control over cell 
 * visit ordering
 */
int aldbro(GRID *);
int wilson(GRID *);
int huntandkill(GRID *);

#endif
