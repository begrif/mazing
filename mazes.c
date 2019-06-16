/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* ways to put a maze in a grid */

#include <stdlib.h>

#include "mazes.h"

/* binary tree maze, iterategrid() call back */ 
int
btreewalker(GRID *g, CELL *c, void*unused)
{
  CELL *cc;
  int edges;
  int go;

  if(!g) { return -1; }
  if(!c) { return -1; }

  edges = edgestatusbycell(g,c);

  if(edges == NORTHEAST_CORNER) {
    return 0;
  } else if(edges & NORTH_EDGE) {
    go = EAST;
  } else if(edges & EAST_EDGE) {
    go = NORTH;
  } else {
    if(random()%2 == 1) {
      go = EAST;
    } else {
      go = NORTH;
    }
  }
  cc = visitdir(g, c, go, ANY);
  connectbycell(c, go, cc, SYMMETRICAL);
  return 0;
} /* btreewalker() */

/* sidewinder tree maze, iterategrid() call back */ 
int
sidewinderwalker(GRID *g, CELL *c, void* status)
{
  sw_tree_status *ts;
  CELL *cc, *oc;
  int edges;
  int closeit;
  int go;
  int nc;

  if(!g) { return -1; }
  if(!c) { return -1; }

  ts = (sw_tree_status*)status;

  if (0 == ts->runlength) {
    ts->runlength = 1;
    ts->runstart_r = c->row;
    ts->runstart_c = c->col;
  } else {
    ts->runlength ++;
  }

  edges = edgestatusbycell(g,c);

  if(edges == NORTHEAST_CORNER) {
    return 0;
  } else if(edges & NORTH_EDGE) {
    closeit = 0;
  } else if(edges & EAST_EDGE) {
    closeit = 1;
  } else {
    closeit = (random()%2 == 1);
  }

  if(closeit) {
    go = NORTH;
    nc = c->col - (random() % ts->runlength);
    oc = visitrc(g, c->row, nc);
    cc = visitdir(g, oc, go, ANY);
    ts->runlength = 0;
  } else {
    go = EAST;
    cc = visitdir(g, c, go, ANY);
    oc = c;
  }
  connectbycell(oc, go, cc, SYMMETRICAL);
  return 0;
} /* sidewinderwalker() */

/* serpentine grid walk, iterategrid() call back */ 
int
serpentine(GRID *g, CELL *c, void*unused)
{
  CELL *cc;
  int edges;
  int go;

  if(!g) { return -1; }
  if(!c) { return -1; }

  edges = edgestatusbycell(g,c);

  if(edges & EAST_EDGE) {
    if(c->row % 2) {
      cc = visitdir(g, c, SOUTH, ANY);
      connectbycell(c, SOUTH, cc, SYMMETRICAL);
    }
  }

  if(edges & WEST_EDGE) {
    if(0 == (c->row % 2)) {
      cc = visitdir(g, c, SOUTH, ANY);
      connectbycell(c, SOUTH, cc, SYMMETRICAL);
    }
  }

  if(!(edges & EAST_EDGE)) {
    cc = visitdir(g, c, EAST, ANY);
    connectbycell(c, EAST, cc, SYMMETRICAL);
  }
  return 0;
} /* serpentine() */

/* a hollow non-maze, iterategrid() call back */ 
int
hollow(GRID *g, CELL *c, void *unused)
{
  CELL *cc;
  int edges;
  int go;

  if(!g) { return -1; }
  if(!c) { return -1; }

  edges = edgestatusbycell(g,c);

  if(!(edges & EAST_EDGE)) {
    cc = visitdir(g, c, EAST, ANY);
    connectbycell(c, EAST, cc, SYMMETRICAL);
  }

  if(!(edges & SOUTH_EDGE)) {
    cc = visitdir(g, c, SOUTH, ANY);
    connectbycell(c, SOUTH, cc, SYMMETRICAL);
  }

  return 0;
} /* hollow() */


/* Named for David Aldous and Andrei Broder, this method cannot
 * use the grid iterator because it needs to visit cells randomly,
 * and it needs to be able to revisit cells.
 * Relies on a grid being marked fully UNVISITED (ctype) to start.
 */
int
aldbro(GRID *g)
{
  CELL *cc, *nc;
  int edges;
  int tovisit;
  int go;

  if(!g) { return -1; }

  cc = visitrandom(g);
  if(!cc) { return -1; }
  cc->ctype = VISITED;
  tovisit = g->max - 1;

  go = NEEDDIR;

  while(tovisit) {

    edges = edgestatusbycell(g,cc);
    
    while( go > FOURDIRECTIONS ) {
      go = FIRSTDIR + (random() % FOURDIRECTIONS);
      if((go == NORTH) && (edges & NORTH_EDGE)) { go = NEEDDIR; }
      if((go == SOUTH) && (edges & SOUTH_EDGE)) { go = NEEDDIR; }
      if((go == WEST ) && (edges &  WEST_EDGE)) { go = NEEDDIR; }
      if((go == EAST ) && (edges &  EAST_EDGE)) { go = NEEDDIR; }
    } /* pick a viable direction */

    nc = visitdir(g, cc, go, ANY);
    if(!nc) { return -1; }

    if(nc->ctype == UNVISITED) {
      nc->ctype = VISITED;
      tovisit --;
      connectbycell(cc, go, nc, SYMMETRICAL);
    }

    cc = nc;
    go = NEEDDIR;

  } /* while cells to visit */

  return 0;
} /* aldbro() */

/*
 * The (David Bruce) Wilson method is a series of random walks, each
 * ending when finding a perviously visited cell. Loops created during
 * the walks are removed before carving the path.
 * Relies on a grid being marked fully UNVISITED (ctype) to start.
 */
int
wilson(GRID *g)
{
  CELL *cc, *nc;
  int edges;
  int tovisit;
  int go;
  TRAIL *walk, trailhead;
  char *notes;

  if(!g) { return -1; }
  notes = (char*) calloc( 1, g->max );

  cc = visitid(g, 0);
  if(!cc) { return -1; }
  cc->ctype = VISITED;
  notes[0] = VISITED;
  tovisit = g->max - 1;

  trailhead.next = NULL;
  trailhead.prev = NULL;

  while(tovisit) {
    int wandering;

    if(tovisit < 0) {
      /* shouldn't happen */
      return -1;
    }

    /* Find somewhere fresh to start the walk */
    while (cc->ctype == VISITED) {
      cc = visitrandom(g);
    }

    notes[cc->id] = WALK_CONSIDER;
    trailhead.cell_id = cc->id;
    walk = &trailhead;
    wandering = 1; 
    go = NEEDDIR;
    
    while ( wandering ) {
      edges = edgestatusbycell(g,cc);
    
      while( go > FOURDIRECTIONS ) {
	go = FIRSTDIR + (random() % FOURDIRECTIONS);
	if((go == NORTH) && (edges & NORTH_EDGE)) { go = NEEDDIR; }
	if((go == SOUTH) && (edges & SOUTH_EDGE)) { go = NEEDDIR; }
	if((go == WEST ) && (edges &  WEST_EDGE)) { go = NEEDDIR; }
	if((go == EAST ) && (edges &  EAST_EDGE)) { go = NEEDDIR; }
      } /* pick a viable direction */

      nc = visitdir(g, cc, go, ANY);
      if(!nc) { return -1; }

      if(notes[nc->id] == WALK_CONSIDER) {
        /* loop detected, backtrack */

	while(walk->cell_id != nc->id) {
	  notes[walk->cell_id] = 0;
	  walk = walk->prev;
	  free(walk->next);
          walk->next = NULL;
	} /* walking back */

	cc = visitid(g, walk->cell_id);
	go = NEEDDIR;
	continue; /* wandering */
      } 
        
      walk->next = (TRAIL*)malloc(sizeof(TRAIL));
      if(!walk->next) { return -1; } /* leaks memory, on out of memory... */
      walk->next->prev = walk;
      walk = walk->next;
      walk->next = NULL;
      walk->cell_id = nc->id;

      if(notes[nc->id] == VISITED) {
        wandering = 0;
      } else {
        notes[nc->id] = WALK_CONSIDER;
	cc = nc;
        go = NEEDDIR;
      }
    } /* while wandering */

    /* We have a trail, walk it back marking visited */
    while ( walk != &trailhead ) {
      tovisit --;
      notes[walk->cell_id] = VISITED;
      nc->ctype = VISITED;

      walk = walk->prev;
      free(walk->next);
      cc = visitid(g, walk->cell_id);
      go = natdirectionbycell(cc, nc);

      connectbycell(cc, go, nc, SYMMETRICAL);
      nc = cc;
    } /* marking the trail */

    notes[walk->cell_id] = VISITED;
    nc->ctype = VISITED;

  } /* while cells to visit */

  return 0;
} /* wilson() */


