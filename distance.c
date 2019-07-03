/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* distance tools for a maze grid */

#include <stdlib.h>
#include <stdio.h>

#include "grid.h"
#include "distance.h"

/* mallocs and initializes the distance map structure to
 * match a particular grid.
 */
DMAP *
createdistancemap(GRID *g, CELL *c)
{
  DMAP *dm;

  if(!g) { return NULL; }
  if(!c) { return NULL; }

  dm = (DMAP *)malloc( sizeof(DMAP) );
  if(!dm) { return NULL; }

  dm->grid = g;
  dm->path = NULL;

  dm->root_id = c->id;
  dm->target_id = NC;
  dm->farthest_id = NC;
  dm->farthest = NV;
  dm->rrow = c->row;
  dm->rcol = c->col;
  dm->msize = g->max;

  dm->map = malloc( g->max * sizeof(int) );
  if(!dm->map) { free(dm); return NULL; }

  dm->frontier = malloc( g->max * sizeof(int) + 1 );
  if(!dm->frontier) { free (dm->map); free(dm); return NULL; }

  for (int m = 0; m < g->max; m++) {  dm->map[m] = NOT_VISITED; }

  dm->frontier[0] = dm->root_id;
  dm->frontier[1] = NV;

  return dm;
} /* createdistancemap() */

/* frees the various bits of a distance map */
void
freedistancemap(DMAP *dm)
{
  TRAIL *walker;

  if(!dm) { return; }
  if(dm->map) { free (dm->map); }
  if(dm->frontier) { free (dm->frontier); }

  walker = dm->path;
  while(walker) {
    TRAIL *goner = walker;
    walker = walker->next;
    free(goner);
  }

  free(dm);
} /* freedistancemap() */


/* This uses Dijkstra's flood-fill method to find a distance.
 * From each cell it tries all other reachable cells until it
 * gets a match. Usually not the fastest way to solve a maze,
 * it is however the most forgiving of weird maze topologies.
 * This must be run (with a distance found) for findpath() to
 * work, findpath() can use lazy results, finding longest
 * possible path, findlongestpath(), needs full results.
 */
int
distanceto(DMAP *dm, CELL *c, int lazy)
{
  int want;
  int of, nf;
  int *frontier;
  CELL *fcell;
  CELL *vcell;
  int far, found;
  int edges, walls;

  if(!dm) { return DISTANCE_ERROR; }
  if(!c) { return DISTANCE_ERROR; }

  want = c->id;

  /* the trivial case */
  if(lazy && (dm->root_id == want)) {
    return 0;
    dm->map[want] = 0;
    dm->target_id = want;
  }

  far = found = 0;

  while( far < dm->msize ) {

    frontier = malloc( dm->msize * sizeof(int) + 1 );
    if(!frontier) { return DISTANCE_ERROR; }
    nf = 0;
    frontier[0] = NV;

    for (of = 0; dm->frontier[of] != NOT_VISITED; of ++) {
      dm->map[dm->frontier[of]] = far;

      if(!lazy) {
        if (far > dm->farthest) {
	  dm->farthest = far;
	  dm->farthest_id = dm->frontier[of];
	}
      }

      fcell = visitid(dm->grid, dm->frontier[of]);
      if(!fcell) {
	free(frontier);
	return DISTANCE_ERROR;
      }

      if(fcell->id == want) {
        dm->target_id = want;
	if(lazy) {
	  free(dm->frontier);
	  dm->frontier = frontier;
	  return far;
	} else {
	  found = 1;
	}
      }

      edges = edgestatusbycell(dm->grid, fcell);
      if(edges == EDGE_ERROR) {
	free(frontier);
	return DISTANCE_ERROR;
      }

      walls = wallstatusbycell(fcell);
      if(walls == WALL_ERROR) {
	free(frontier);
	return DISTANCE_ERROR;
      }
      
      walls |= edges;

      for(int go = FIRSTDIR; go < FOURDIRECTIONS; go ++) {
	int check = 0;
	if(go == NORTH) { check = walls&NORTH_WALL; }
	if(go == SOUTH) { check = walls&SOUTH_WALL; }
	if(go == WEST ) { check = walls&WEST_WALL; }
	if(go == EAST ) { check = walls&EAST_WALL; }
	if(! check) {
	  vcell = visitdir(dm->grid, fcell, go, ANY);
	  if(! vcell ) {
	    free(frontier);
	    return DISTANCE_ERROR;
	  }

	  /* only add it if we haven't seen it already */
	  if(dm->map[vcell->id] == NOT_VISITED) {
	    if(dm->map[vcell->id] != FRONTIER) {
	      frontier[nf++] = vcell->id;
	      dm->map[vcell->id] = FRONTIER;
	    }
	  }
	} /* for wall check */
      } /* for direction */

      frontier[nf] = NV;

    } /* for id in frontier */
    far ++;
    free(dm->frontier);
    dm->frontier = frontier;

    // printf("\nDebug round %d\n", far);
    // ascii_dmap(dm);

  } /* while walking as far as possible */

  if (found) { 
    return 0;
  }
  return(DISTANCE_ERROR);
} /* distanceto() */

int
findpath(DMAP *dm)
{
  TRAIL *walk;
  TRAIL *step;
  int id, sid, curdis;
  int si, sj;

  if(!dm) { return DISTANCE_ERROR; }

  /* this is the case when distanceto() wasn't run, or failed. */
  if(dm->target_id < 0) { return DISTANCE_ERROR; }

  walk = (TRAIL *)malloc( sizeof(TRAIL) );
  if(!walk) { return DISTANCE_ERROR; }
  
  walk->cell_id = id = dm->target_id;
  walk->next = NULL;
  walk->prev = NULL;

  while( (curdis = dm->map[id]) ) {
    step = (TRAIL *)malloc( sizeof(TRAIL) );
    /* this leaks memory, but only when malloc fails */
    if(!step) { return DISTANCE_ERROR; }
     
    step->next = walk;
    step->prev = NULL;
    walk = step;

    /* At least one neighbor should be curdis - 1,
     * but there might be multiple equally short paths.
     */
    si = id / ( dm->grid->cols );
    sj = id % ( dm->grid->cols );

#define TEST_SID \
	if(curdis == dm->map[sid] + 1) { \
	  if(isconnectedbyid(dm->grid, id, sid, ANYDIR) != NC) { \
	    walk->cell_id = id = sid; \
	    continue; \
	  } \
	}

    if(si) {
      sid = id - dm->grid->cols;
      TEST_SID;
    }
    if(sj) {
      sid = id - 1;
      TEST_SID;
    }
    if(si < (dm->grid->rows - 1)) {
      sid = id + dm->grid->cols;
      TEST_SID;
    }
    if(sj < (dm->grid->cols - 1)) {
      sid = id + 1;
      TEST_SID;
    }

    /* this shouldn't be reached */
    walk->cell_id = id = NV;
    break;
  }

  dm->path = walk;
  if(curdis || id == NV) {
    return DISTANCE_ERROR;
  }

  return 0;
} /* findpath() */

/* Finds the longest path possible in a maze (or a longest, if there
 * are ties). Since on some grids some cells are not part of a maze,
 * eg a mask is used, a cell type to start the hunt from is needed.
 * The type must correspond to at least two cells that are connected
 * to the rest of the maze, which will be the starting and ending points
 * for the first pass flood fill. There's no gaurrantee either will be
 * on the final longest path.
 */
DMAP *
findlongestpath(GRID *g, int t)
{
  DMAP *first, *second;
  CELL *pa, *pb;
  int hid, rc, fid;

  if(!g) {
    return NULL;
  }

  /* Hunt from NW for first cell */
  hid = 0;
  do {
    pa = visitid(g, hid);
    if(!pa) { return NULL; }
    hid ++;
  } while (pa->ctype != t);

  /* Hunt from SE for first cell */
  hid = g->max - 1;
  do {
    pb = visitid(g, hid);
    if(!pb) {
      return NULL;
    }
    hid --;
  } while (pb->ctype != t);


  first = createdistancemap(g, pa);
  if(!first) {
    return NULL;
  }

  rc = distanceto(first, pb, NONLAZYMAP);
  if(rc == DISTANCE_ERROR) {
    freedistancemap(first);
    return NULL;
  }

  fid = first->farthest_id;
  if(fid == 0) {
    /* Point A was id 0, if the furthest point from Point A is
     * Point A, we've got a real degenerate case.
     */
    TRAIL *walk = (TRAIL *)malloc( sizeof(TRAIL) );
    walk->next = walk->prev = NULL;
    walk->cell_id = fid;
    first->path = walk;
    return first;
  }

  freedistancemap(first);
  pb = visitid(g, fid);
  if(!pb) {
    return NULL;
  }

  second = createdistancemap(g, pb);
  if(!second) {
    return NULL;
  }

  rc = distanceto(second, pa, NONLAZYMAP);
  if(rc == DISTANCE_ERROR) {
    freedistancemap(second);
    return NULL;
  }

  second->target_id = second->farthest_id;
  rc = findpath(second);
  if(rc == DISTANCE_ERROR) {
    freedistancemap(second);
    return NULL;
  }

  return second;
} /* findlongestpath() */

int
iteratewalk(DMAP *dm, int(*ifunc)(DMAP *, int, void*), void*param)
{
  TRAIL *step;
  int sum = 0;

  if(!dm) {
    return -1;
  }
  if(!ifunc) {
    return -1;
  }
  if(!dm->path) {
    return -1;
  }

  step = dm->path;
  do {
    sum += ifunc(dm, step->cell_id, param);
    step = step->next;
  } while(step);
  
  return sum;
} /* iteratewalk() */

static
int
namewalker(DMAP *dm, int cid, void*param)
{
  char scratch[BUFSIZ];
  
  if(param) {
    namebyid(dm->grid, cid, (char*)param);
  } else {
    snprintf(scratch, BUFSIZ, "%d", dm->map[cid]);
    namebyid(dm->grid, cid, scratch);
  }
  return 0;
} /* namewalker() */

/* Insert a name into all of the steps on a path
 * if name is NULL stringify the step count, and use that.
 * The three names are first (first step only), middle,
 * and last (last step only).
 * Returns a negative value on error, zero on success.
 */
int
namepath(DMAP *dm, char *fname, char *mname, char *lname)
{
  int rc;

  rc = iteratewalk(dm, namewalker, mname);
  if(rc < 0) { return rc; }

  if(fname) {
    namebyid(dm->grid, dm->root_id, fname);
  }
  if(lname) {
    namebyid(dm->grid, dm->target_id, lname);
  }
} /* namepath */

/* print the distance map for testing */
void
ascii_dmap(DMAP *dm)
{
  int i, j, f, d;
  
  if(!dm) { return; }

  f = 0;
  for(i = 0; i < dm->grid->rows; i++) {
    for(j = 0; j < dm->grid->cols; j++) {
      d = dm->map[f++];
      if(d == NOT_VISITED) {
	printf("unk ");
      } else if(d == FRONTIER) {
	printf("... ");
      } else {
	printf("%3d ", d);
      }
    }
    putchar('\n');
  }
} /* ascii_dmap() */

