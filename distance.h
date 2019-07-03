#ifndef _DISTANCE_H
#define _DISTANCE_H

#define LAZYMAP            1
#define NONLAZYMAP         0
#define DISTANCE_NOPATH   -1
#define DISTANCE_ERROR    -2

#define NV                -2	/* no value or not visited */
#define NOT_VISITED       NV
#define FRONTIER          -3

/* your typical double linked list for holding a pathway through a maze */
typedef struct trail_t {
  int cell_id;
  struct trail_t *next;
  struct trail_t *prev;
} TRAIL;

/* no part of this structure is intended to be changed by maze generators */
typedef struct {
  GRID *grid;
  int root_id;		/* set at creation time */
  int target_id;	/* set when target found */
  int farthest_id;	/* set with non-lazy distance maps */
  int farthest;		/* set with non-lazy distance maps */
  int rrow, rcol;	/* root row, col values, set at creation time */
  int msize;		/* size of map; frontier always one larger */
  int *map;		/* distances from root, indexed by cell id */
  int *frontier;	/* cells to check when looking for a target */
  TRAIL *path;		/* linked list of a path from root to target */
} DMAP;


DMAP *createdistancemap(GRID *, CELL *);
void freedistancemap(DMAP *);

int distanceto(DMAP *, CELL *,int /* lazy flag */);
int findpath(DMAP *);
DMAP *findlongestpath(GRID *, int /*celltype*/);

int iteratewalk(DMAP *, int(*)(DMAP *, int, void*), void*);
int namepath(DMAP *, char */*first*/, char */*middle*/, char*/*last*/);

void ascii_dmap(DMAP *);
#endif
