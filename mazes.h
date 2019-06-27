/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* ways to put a maze in a grid */

#ifndef _MAZES_H
#define _MAZES_H

#include <sys/queue.h>

#include "grid.h"
#include "distance.h"

/* suggested meanings of cell types */
#define UNVISITED	1
#define VISITED		2
#define MASKED		17

#define WALK_CONSIDER   3	/* used in wilson's notes[] */

/* greater than any valid direction */
#define NEEDDIR		(99+DIRECTIONS)

typedef struct masksetting_s {
  int type_unvisited;
  int type_visited;
  int type_masked;
  int to_visit;
} MASKSETTING;

/* for hollow */
#define HMODE_ALL		0
#define HMODE_SAME_AS		1
#define HMODE_SAME_AS_STRICT	2
#define HMODE_DIFFERENT		3
#define HMODE_DIFFERENT_STRICT	4

typedef struct hollow_config_s {
  int mode;
  int ctype;
} HOLLOWCONFIG;

/* for sidewinder */
typedef struct {
  int runlength;
  int runstart_r;
  int runstart_c;
} sw_tree_status;

/* for backtracker */
typedef struct backtrack_s {
  CELL *cell;
  SLIST_ENTRY(backtrack_s) trail;
} backtrack_stack_t;

/* iterategrid() call backs; these can generate a "maze" by visiting
 * every cell once in any order.
 */
int btreewalker(GRID *, CELL *, void *);
int sidewinderwalker(GRID *, CELL *, void *);
int serpentine(GRID *, CELL *, void *);

/* by default, hollow clears all walls. Using NULL for the user config
 * pointer gets that behavior, or if a HOLLOWCONFIG with mode set to
 * HMODE_ALL is passed it, you get that behavior.
 * If a HOLLOWCONFIG is used with other options, things change.
 *
 *    HMODE_SAME_AS     HMODE_SAME_AS_STRICT
 * With SAME_AS, cells the same type as ctype are hollowed out. With
 * STRICT, the connections will go to non-ctype cells adjacent, without
 * it, only connections between like types are done.
 *
 *     HMODE_DIFFERENT  HMODE_DIFFERENT_STRICT
 * This behavior is analogous to SAME_AS, but in the other direction:
 * cells different from ctype are hollowed out, with STRICT no connections
 * will be made to ctype cells, without it, non-ctype will connect to
 * to edge ctype cells.
 */
int hollow(GRID *, CELL *, HOLLOWCONFIG *);

/* non-iterator generators; these need more control over cell 
 * visit ordering
 */
int aldbro(GRID *, MASKSETTING *);
int wilson(GRID *, MASKSETTING *);
int huntandkill(GRID *, MASKSETTING *);
int backtracker(GRID *, MASKSETTING *);

/* helper(s) */
void defaultmasksetting(MASKSETTING *);
#endif
