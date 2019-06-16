/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* ways to put a maze in a grid */

#include <stdlib.h>

/* A macro system for several simple data structures, available on
 * BSD and Linux systems (and BSD in origin). The singlely-linked
 * list (SLIST_ family) of macros will be used to implement a stack.
 * SLIST_INSERT_HEAD() works like push() and SLIST_FIRST()/SLIST_REMOVE_HEAD()
 * paired work like pop(). malloc() your items to push, free() your pop'ed.
 */
#include <sys/queue.h>

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

/*
 * Hunt-and-kill alternates hunting for unseen spaces and killing
 * them with a random walk ending when finding a perviously visited
 * cell. 
 * Relies on a grid being marked fully UNVISITED (ctype) to start.
 */
int
huntandkill(GRID *g)
{
  CELL *cc, *nc;
  int edges;
  int tovisit;
  int go, dir;

  if(!g) { return -1; }

  /* first hunt is the easiest */
  cc = visitrandom(g);
  if(!cc) { return -1; }
  cc->ctype = VISITED;
  tovisit = g->max - 1;

  go = NEEDDIR;

  while(tovisit) {

    /* now some kills */

    edges = edgestatusbycell(g,cc);
    
    dir = (random() % FOURDIRECTIONS);
    for (int a = 0; a < 4; a ++) {
      go = FIRSTDIR + (dir + a) % 4;
      if((go == NORTH) && (edges & NORTH_EDGE)) { continue; }
      if((go == SOUTH) && (edges & SOUTH_EDGE)) { continue; }
      if((go == WEST ) && (edges &  WEST_EDGE)) { continue; }
      if((go == EAST ) && (edges &  EAST_EDGE)) { continue; }

      nc = visitdir(g, cc, go, ANY);
      if(!nc) { return -1; }
      if(nc->ctype == UNVISITED) { break; }
    } /* pick a good direction */


    /* this might fail if we tried all of the directions and found none
     * unvisited
     */
    if(nc->ctype == UNVISITED) {
      nc->ctype = VISITED;
      tovisit --;
      connectbycell(cc, go, nc, SYMMETRICAL);
      cc = nc;
      go = NEEDDIR;

      continue; /* no need to go hunting */
    }

    /* the new cell (nc) was already dead, let's hunt for a
     * different one.
     */
    if(tovisit) {
      int id;
      for (id = 0; id < g->max; id ++) {
        cc = visitid(g,id);
	if( cc->ctype == UNVISITED ) {
	  if( ncountbycell(g, cc, OF_TYPE, VISITED) ) {
	    /* found a suitable cell:
	     * not previously visited
	     * next to a visited
	     */
	    cc->ctype = VISITED;
	    tovisit --;
	    break;
	  }
	}
      } /* finding a cell */

      /* have a suitable current cell (cc)
       * find one of the visited neighbors and join them
       */
      dir = (random() % FOURDIRECTIONS);
      edges = edgestatusbycell(g,cc);
      for (int a = 0; a < 4; a ++) {
	go = FIRSTDIR + (dir + a) % 4;
	if((go == NORTH) && (edges & NORTH_EDGE)) { continue; }
	if((go == SOUTH) && (edges & SOUTH_EDGE)) { continue; }
	if((go == WEST ) && (edges &  WEST_EDGE)) { continue; }
	if((go == EAST ) && (edges &  EAST_EDGE)) { continue; }

        nc = visitdir(g, cc, go, ANY);
	if(nc && (nc->ctype == VISITED) ) {
          connectbycell(cc, go, nc, SYMMETRICAL);
	  break;
	} 
      } /* pick a random dir and (if okay) make a link */
    } /* still have something to visit (hunt phase) */

    go = NEEDDIR;
  } /* while cells to visit (main loop) */

  return 0;
} /* huntandkill() */


/*
 * This backtracker is rather like hunt-and-kill, but retreats along
 * it's own path when it needs to find a new starting point.
 * Relies on a grid being marked fully UNVISITED (ctype) to start.
 */
int
backtracker(GRID *g)
{
  CELL *cc, *nc;
  SLIST_HEAD(bthead_s, backtrack_s) stack;
  backtrack_stack_t *step;

  int edges;
  int tovisit;
  int go, dir;

  if(!g) { return -1; }
  SLIST_INIT(&stack);


  /* start anywhere */
  cc = visitrandom(g);
  if(!cc) { return -1; }
  tovisit = g->max - 1;
  cc->ctype = VISITED;

  while(tovisit) {

    /* let's wander */

    edges = edgestatusbycell(g,cc);

    step = (backtrack_stack_t*) malloc( sizeof(backtrack_stack_t) );
    step->cell  = cc;

    /* push */
    SLIST_INSERT_HEAD(&stack, step, trail);

    dir = (random() % FOURDIRECTIONS);
    for (int a = 0; a < 4; a ++) {
      go = FIRSTDIR + (dir + a) % 4;
      if((go == NORTH) && (edges & NORTH_EDGE)) { continue; }
      if((go == SOUTH) && (edges & SOUTH_EDGE)) { continue; }
      if((go == WEST ) && (edges &  WEST_EDGE)) { continue; }
      if((go == EAST ) && (edges &  EAST_EDGE)) { continue; }

      nc = visitdir(g, cc, go, ANY);
      if(!nc) { return -1; }
      if(nc->ctype == UNVISITED) { break; }
    } /* pick a good direction */


    /* this might fail if we tried all of the directions and found none
     * unvisited
     */
    if(nc->ctype == UNVISITED) {
      nc->ctype = VISITED;
      tovisit --;
      connectbycell(cc, go, nc, SYMMETRICAL);
      cc = nc;

      continue; /* wander some more */
    }

    /* the new cell (nc) was a dead end, backtrack */
    if(tovisit) {
      while(!SLIST_EMPTY(&stack)) {
	/* pop */
        step = SLIST_FIRST(&stack); SLIST_REMOVE_HEAD(&stack, trail);
	cc = step->cell;
	free(step);
	if( ncountbycell(g, cc, OF_TYPE, UNVISITED) ) {
	  break;
	}

      }
    } /* still have something to visit (backtrack phase) */

  } /* while cells to visit (main loop) */

  /* clean up rest of stack */
  while(!SLIST_EMPTY(&stack)) {
    /* pop */
    step = SLIST_FIRST(&stack); SLIST_REMOVE_HEAD(&stack, trail);
    free(step);
  }
    
  return 0;
} /* backtracker() */
