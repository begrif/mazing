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
hollow(GRID *g, CELL *c, HOLLOWCONFIG *hmode)
{
  CELL *cc;
  int edges;
  int go;
  int mode;
  int t;

  if(!g) { return -1; }
  if(!c) { return -1; }

  if(!hmode) {
    mode = HMODE_ALL;
    t    = -1; /* unused anyway */
  } else {
    mode = hmode->mode;
    t    = hmode->ctype;
  }

  edges = edgestatusbycell(g,c);

  if(!(edges & EAST_EDGE)) {
    cc = visitdir(g, c, EAST, ANY);

    if((!mode)                                        || /* all */
       ((mode == HMODE_SAME_AS) && (c->ctype == t))   ||
       ((mode == HMODE_DIFFERENT) && (c->ctype != t)) ||
       ((mode == HMODE_SAME_AS_STRICT) &&
       		(c->ctype == t) && (cc->ctype == t))  ||
       ((mode == HMODE_DIFFERENT_STRICT) &&
       		(c->ctype != t) && (cc->ctype != t))    ) {
      connectbycell(c, EAST, cc, SYMMETRICAL);
    }
  }

  if(!(edges & SOUTH_EDGE)) {
    cc = visitdir(g, c, SOUTH, ANY);

    if((!mode)                                        || /* all */
       ((mode == HMODE_SAME_AS) && (c->ctype == t))   ||
       ((mode == HMODE_DIFFERENT) && (c->ctype != t)) ||
       ((mode == HMODE_SAME_AS_STRICT) &&
       		(c->ctype == t) && (cc->ctype == t))  ||
       ((mode == HMODE_DIFFERENT_STRICT) &&
       		(c->ctype != t) && (cc->ctype != t))    ) {
      connectbycell(c, SOUTH, cc, SYMMETRICAL);
    }
  }

  /* with the loose type checks, we can't rely on SYMMETRICAL
   * links to get them all with just south and east visits.
   */
  if((mode == HMODE_SAME_AS) || (mode == HMODE_DIFFERENT)) {
    if(!(edges & WEST_EDGE)) {
      cc = visitdir(g, c, WEST, ANY);

      if(((mode == HMODE_SAME_AS) && (c->ctype == t))   ||
         ((mode == HMODE_DIFFERENT) && (c->ctype != t))) {
        connectbycell(c, WEST, cc, SYMMETRICAL);
      }
    }

    if(!(edges & NORTH_EDGE)) {
      cc = visitdir(g, c, NORTH, ANY);

      if(((mode == HMODE_SAME_AS) && (c->ctype == t))   ||
         ((mode == HMODE_DIFFERENT) && (c->ctype != t))) {
        connectbycell(c, NORTH, cc, SYMMETRICAL);
      }
    }
  } /* loose hollow modes */

  return 0;
} /* hollow() */


/* Named for David Aldous and Andrei Broder, this method cannot
 * use the grid iterator because it needs to visit cells randomly,
 * and it needs to be able to revisit cells.
 * Relies on a grid being marked fully UNVISITED (ctype) to start.
 *
 * Cells originally not marked UNVISITED (eg MASKED) will not be
 * part of the maze. When using a mask, it is important to pass
 * in a count of cells to visit.
 *
 * The masksetting can change the default values for VISITED and
 * UNVISITED, it needs to be provided (and to_count must be set) when
 * using a mask.
 */
int
aldbro(GRID *g, MASKSETTING *ms)
{
  CELL *cc, *nc;
  int edges;
  int tovisit;
  int visited;
  int unvisited;
  int masked;
  int nid;
  int go;

  if(!g) { return -1; }

  if(ms) {
    unvisited = ms->type_unvisited;
    visited   = ms->type_visited;
    masked    = ms->type_masked;
    tovisit   = ms->to_visit;
  } else {
    unvisited = UNVISITED;
    visited   = VISITED;
    masked    = MASKED;
    tovisit   = 0;
  }

  if(tovisit < 1) { tovisit = g->max; }

  cc = visitrandom(g);
  if(!cc) { return -1; }
  nid = cc->id;
  while( cc->ctype != unvisited ) {
    nid ++;
    cc = visitid(g, nid % g->max );
    if(!cc) { return -1; }
    if( nid > (2 * g->max) ) {
      /* don't loop forever */
      return -1;
    }
  }
  cc->ctype = visited;
  tovisit --;

  go = NEEDDIR;

  while(tovisit) {

    edges = edgestatusbycell(g,cc);
    
    while( go > FOURDIRECTIONS ) {
      go = FIRSTDIR + (random() % FOURDIRECTIONS);
      if((go == NORTH) && (edges & NORTH_EDGE)) { go = NEEDDIR; }
      if((go == SOUTH) && (edges & SOUTH_EDGE)) { go = NEEDDIR; }
      if((go == WEST ) && (edges &  WEST_EDGE)) { go = NEEDDIR; }
      if((go == EAST ) && (edges &  EAST_EDGE)) { go = NEEDDIR; }

      if (go < NEEDDIR) {
        nc = visitdir(g, cc, go, ANY);
        if(!nc) { return -1; }
        if(nc->ctype == MASKED) {                 go = NEEDDIR; }
      }
    } /* pick a viable direction */


    if(nc->ctype == unvisited) {
      nc->ctype = visited;
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
 *
 * Cells originally not marked UNVISITED (eg MASKED) will not be
 * part of the maze. When using a mask, it is important to pass
 * in a count of cells to visit.
 *
 * The masksetting can change the default values for VISITED and
 * UNVISITED, it needs to be provided (and to_count must be set) when
 * using a mask. The MASKED value isn't actually used here.
 */
int
wilson(GRID *g, MASKSETTING *ms)
{
  CELL *cc, *nc;
  int edges;
  int tovisit;
  int visited;
  int wconsider;
  int unvisited;
  int go;
  int nid;
  int wandering;
  TRAIL *walk, trailhead;
  char *notes;

  if(!g) { return -1; }
  notes = (char*) calloc( 1, g->max );

  if(!notes) { return -1; }

  if(ms) {
    unvisited = ms->type_unvisited;
    visited   = ms->type_visited;
    wconsider = ms->type_masked;
    tovisit   = ms->to_visit;
  } else {
    unvisited = UNVISITED;
    visited   = VISITED;
    wconsider = WALK_CONSIDER;
    tovisit   = 0;
  }

  if(tovisit < 1) { tovisit = g->max; }

  nid = 0;
  do {
    cc = visitid(g, nid);
    if(!cc) { return -1; }
    nid ++;
    if(nid == g->max) {
      /* ran out of ids */
      return -1; 
    }
  } while( cc->ctype != unvisited );

  cc->ctype = visited;
  notes[cc->id] = visited;
  tovisit --;

  trailhead.next = NULL;
  trailhead.prev = NULL;

  while(tovisit) {

    if(tovisit < 0) {
      /* shouldn't happen */
      return -1;
    }

    /* Find somewhere fresh to start the walk */
    do {
      cc = visitrandom(g);
    } while (cc->ctype != unvisited);

    notes[cc->id] = wconsider;
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

	if (go < NEEDDIR) {
	  nc = visitdir(g, cc, go, ANY);
	  if(!nc) { return -1; }
	  if(nc->ctype == MASKED) {                 go = NEEDDIR; }
	}
      } /* pick a viable direction */

      if(notes[nc->id] == wconsider) {
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

      if(notes[nc->id] == visited) {
        wandering = 0;
      } else {
        notes[nc->id] = wconsider;
	cc = nc;
        go = NEEDDIR;
      }
    } /* while wandering */

    /* We have a trail, walk it back marking visited */
    while ( walk != &trailhead ) {
      tovisit --;
      notes[walk->cell_id] = visited;
      nc->ctype = visited;

      walk = walk->prev;
      free(walk->next);
      cc = visitid(g, walk->cell_id);
      go = natdirectionbycell(cc, nc);

      connectbycell(cc, go, nc, SYMMETRICAL);
      nc = cc;
    } /* marking the trail */

    notes[walk->cell_id] = visited;
    nc->ctype = visited;

  } /* while cells to visit */

  return 0;
} /* wilson() */

/*
 * Hunt-and-kill alternates hunting for unseen spaces and killing
 * them with a random walk ending when finding a perviously visited
 * cell. 
 *
 * Relies on a grid being marked fully UNVISITED (ctype) to start.
 *
 * The masksetting can change the default values for VISITED and
 * UNVISITED, it needs to be provided (and to_count must be set) when
 * using a mask. The MASKED value isn't actually used here.
 */
int
huntandkill(GRID *g, MASKSETTING *ms)
{
  CELL *cc, *nc;
  int edges;
  int unvisited;
  int visited;
  int masked;
  int tovisit;
  int nid;
  int go, dir;

  if(!g) { return -1; }

  if(ms) {
    unvisited = ms->type_unvisited;
    visited   = ms->type_visited;
    masked    = ms->type_masked;
    tovisit   = ms->to_visit;
  } else {
    unvisited = UNVISITED;
    visited   = VISITED;
    masked    = MASKED;
    tovisit   = 0;
  }

  if(tovisit < 1) { tovisit = g->max; }

  /* first hunt is the easiest */
  cc = visitrandom(g);
  if(!cc) { return -1; }

  nid = cc->id;
  while( cc->ctype != unvisited ) {
    nid ++;
    cc = visitid(g, nid % g->max );
    if(!cc) { return -1; }
    if( nid > (2 * g->max) ) {
      /* don't loop forever */
      return -1;
    }
  }
  cc->ctype = visited;
  tovisit --;

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
      if(nc->ctype == unvisited) { break; }
    } /* pick a good direction */


    /* this might fail if we tried all of the directions and found none
     * unvisited
     */
    if(nc->ctype == unvisited) {
      nc->ctype = visited;
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
	if( cc->ctype == unvisited ) {
	  if( ncountbycell(g, cc, OF_TYPE, visited) ) {
	    /* found a suitable cell:
	     * not previously visited
	     * next to a visited
	     */
	    cc->ctype = visited;
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
	if(nc && (nc->ctype == visited) ) {
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
 *
 * Relies on a grid being marked UNVISITED (ctype) to start.
 *
 * Cells originally not marked UNVISITED (eg MASKED) will not be
 * part of the maze. When using a mask, it is important to pass
 * in a count of cells to visit.
 *
 * The masksetting can change the default values for VISITED and
 * UNVISITED, it needs to be provided (and to_count must be set) when
 * using a mask. The MASKED value isn't actually used here.
 */
int
backtracker(GRID *g, MASKSETTING *ms)
{
  CELL *cc, *nc;
  SLIST_HEAD(bthead_s, backtrack_s) stack;
  backtrack_stack_t *step;
  int unvisited;
  int visited;
  int masked;
  int tovisit;
  int edges;
  int nid;
  int go, dir;

  if(!g) { return -1; }
  SLIST_INIT(&stack);

  if(ms) {
    unvisited = ms->type_unvisited;
    visited   = ms->type_visited;
    masked    = ms->type_masked;
    tovisit   = ms->to_visit;
  } else {
    unvisited = UNVISITED;
    visited   = VISITED;
    masked    = MASKED;
    tovisit   = 0;
  }

  if(tovisit < 1) { tovisit = g->max; }


  /* start anywhere */
  cc = visitrandom(g);
  if(!cc) { return -2; }

  nid = cc->id + 1;
  while( cc->ctype != unvisited ) {
    cc = visitid(g, nid % g->max );
    if(!cc) { return -1; }
    nid ++;
    if( nid > (2 * g->max) ) {
      /* don't loop forever */
      return -3;
    }
  }
  cc->ctype = visited;
  tovisit --;

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
      if(!nc) { return -4; }
      if(nc->ctype == unvisited) { break; }
    } /* pick a good direction */


    /* this might fail if we tried all of the directions and found none
     * unvisited
     */
    if(nc->ctype == unvisited) {
      nc->ctype = visited;
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
	if( ncountbycell(g, cc, OF_TYPE, unvisited) ) {
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


void
defaultmasksetting(MASKSETTING *ms)
{
  if(ms) {
    ms->type_unvisited = UNVISITED;
    ms->type_visited   = VISITED;
    ms->type_masked    = MASKED;
    ms->to_visit       = 0;
  }
} /* defaultmasksetting() */
