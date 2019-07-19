/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* tools for a maze grid */

/* get us some strnlen */
#define _POSIX_C_SOURCE  200809L
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "grid.h"


/* initializes a cell to have no connections
 * be of type t
 * be at row i, col j
 * and have id id
 */
void
initcell(CELL *c, int t, int i, int j, int id)
{
  if(!c) {return;}

  /* calloc()ed whole grid  memset((void*)c, 0, sizeof(CELL)); */
  c->id = id;
  c->row = i;
  c->col = j;
  c->ctype = t;
  c->name = NULL;
  c->data = NULL;

  for(int d = 0; d < DIRECTIONS; d++) {
    c->dir[d] = NC;
  }

} /* initcell() */


/* frees pointers within a cell */
void
freecell(CELL *c)
{
  if(!c) {return;}

  if(c->name) { free(c->name); }
  if(c->data) { free(c->data); }
} /* freecell() */


/* Copy a cell: id, position, type, name, direction data, and optionally
 * the pointer to the user data (it's opaque, so we can't actually copy
 * the contents). Returns 0 on success, negative on failure.
 * The CELLCOPYCONFIG is used to remap cell IDs and row/col data if
 * needed. For copygrid() not needed, for pasteintogrid() almost
 * certainly is needed.
 */
int
copycell(CELL *orig, CELL *dupe, CELLCOPYCONFIG *conf)
{
  int r,c,newid;

#define CALCNEWID(i) { if((i == NC) || (conf->origwidth == conf->newwidth) && \
                          (conf->rowoffset == 0) && \
                          (conf->coloffset == 0)) { \
			 newid = (i); \
		       } else { \
			 r = ((i) / conf->origwidth) + conf->rowoffset; \
			 c = ((i) % conf->origwidth) + conf->coloffset; \
			 newid = (r * conf->newwidth) + c; \
		       } \
		     }  

  CALCNEWID(orig->id);
  dupe->id = newid;
  dupe->row = conf->rowoffset + orig->row;
  dupe->col = conf->coloffset + orig->col;
  dupe->ctype = orig->ctype;

  for(int d = 0; d < DIRECTIONS; d++) {
    CALCNEWID(orig->dir[d]);
    dupe->dir[d] = newid;
  }

  if(orig->name) {
    /* namebycell can return 0 or 1 for two different success states */
    if(0 < namebycell(dupe, orig->name)) {
      return -1;
    }
  }

  if(conf->includeuserdata) {
    dupe->data = orig->data;
  }

  return 0;
} /* copycell() */


/* creates a block of cells, used in creategrid() and grid modifiers */
static
CELL *
createcells(int rows, int cols, int t, int initcells)
{
  CELL *block;
  CELL *c;
  int count = rows * cols;

  block = (CELL*)calloc((size_t)count, sizeof(CELL));
  if(!block) {
    return (CELL*)NULL;
  }

  if(initcells) {
    count = 0;
    for (int row = 0; row < rows; row ++) {
      for (int col = 0; col < cols; col ++) {
	c = &(block[count]);
	initcell(c, t, row, col, count);
	count ++;
      }
    }
  }
  return block;
} /* createcells() */


GRID*
creategrid(int i, int j, int t)
{
  GRID *g;
  int count;
  static int grid_ran_srandom = 0;

  if((i < 1) || (j < 1)) {
    return (GRID*)NULL;
  }
  
  g = (GRID*)calloc(1, sizeof(GRID));
  if(!g) { return g; }

  count = i*j;
  g->rows = i;
  g->cols = j;
  g->planes = 1;	/* up/down later */
  g->gtype = t;
  g->data = NULL;
  g->name = NULL;
  g->max = count;
  g->cells = createcells(g->rows, g->cols, t, 1);
  if(!g->cells) {
    free(g);
    return (GRID*)NULL;
  }

  if(!grid_ran_srandom) {
    grid_ran_srandom = (int)time(NULL);
    srandom(grid_ran_srandom);
  }

  return g;
} /* creategrid() */


void
freegrid(GRID* g)
{
  int id;
  if(!g) {return;}

  if(g->name) { free(g->name); }
  if(g->data) { free(g->data); }

  if(g->cells) {
    for(id = 0; id < g->max; id++) {
      freecell(visitid(g,id));
    }
    free(g->cells);
  }

  free(g);
} /* freegrid() */


/* Perform a deep copy of an entire grid. If includeuserdata is set
 * the pointers for the user data will be copied (since userdata is
 * opaque, we can't copy the actual contents).
 */
GRID *
copygrid(GRID *g, int includeuserdata)
{
  GRID *dupe;
  int id;
  CELL *oc, *dc;
  CELLCOPYCONFIG conf;

  if(!g) { return NULL; }

  dupe = creategrid(g->rows, g->cols, g->gtype);
  if(!dupe) { return NULL; }

  if(g->name) {
    /* namegrid() can return 1 and be successful, but only when free()ing
     * an existing name; our dupe won't have one of those.
     */
    if(0 != namegrid(dupe, g->name)) {
      freegrid(dupe);
      return NULL;
    }
  }

  if(includeuserdata) {
    dupe->data = g->data;
  }

  conf.includeuserdata = includeuserdata;
  conf.origwidth       = g->cols;
  conf.newwidth        = g->cols;
  conf.rowoffset       = 0;
  conf.coloffset       = 0;

  for (id = 0; id < g->max; id++) {
    oc = &(g->cells[id]);
    dc = &(dupe->cells[id]);

    if(0 != copycell(oc, dc, &conf)) {
      freegrid(dupe);
      return NULL;
    }
  }

  return dupe;
} /* copygrid() */


/* Paste source grid sg into destination grid dg offset top rows south
 * and left rows east, optionally copying the pointers to the userdata.
 * The source grid is not modified in anyway. Connections around the
 * edges of the paste area are not modified, so if they had existed,
 * they are probably going to be one way connections afterwards.
 *
 * Returns 0 on success and a negative value on failure.
 */
int
pasteintogrid(GRID *sg, GRID *dg, int top, int left, int includeuserdata)
{
  int i, j, id, rc;
  CELL *sc, *dc;
  CELLCOPYCONFIG conf;

  /* flat out bad parameters */
  if(!dg)      { return -1; }
  if(!sg)      { return -1; }
  if(top  < 0) { return -1; }
  if(left < 0) { return -1; }

  /* more subtly bad options */
  if(sg->cols + left > dg->cols) { return -2; }
  if(sg->rows + top  > dg->rows) { return -2; }

  conf.includeuserdata = includeuserdata;
  conf.origwidth       = sg->cols;
  conf.newwidth        = dg->cols;
  conf.rowoffset       = top;
  conf.coloffset       = left;
  
  /* rc : return code
   * sc : source cell
   * dc : destination cell
   */
  rc = 0;
  for (i = 0; i < sg->cols; i++) {
    for (j = 0; j < sg->rows; j++) {
      sc = visitrc(sg, i    , j     );
      dc = visitrc(dg, i+top, j+left);
      /* copycell returns 0 on success, and -1 on error */
      rc += copycell(sc, dc, &conf);
    }
  } /* copy the cells */

  return rc;
} /* pasteintogrid() */

/* For a given grid, create a labyrinth (unicursal maze: there are no
 * branches or dead ends, just a long winding path all the way through).
 *
 * Optionally any edge cell can be an entrance by having connections
 * outside the grid in the entrance direction. Three options there:
 *   1. A cell can connect back to itself in that direction
 *   2. A cell can link to a cell ID < NC
 *   3. A cell can link to a cell ID >= grid max
 * If an entrance ID is specified, the links for the new cells that
 * go off-grid will be fixed. Case one cells will link back to the
 * newly generated cells; case 2 and 3 cells will link to the same
 * cell ID as the original. Be careful with case 3, since the new
 * grid has a 4x larger max.
 *
 * Cell types, but not names, will be copied to children cells.
 *
 * The labyrinth is twice as wide and tall as the original grid, and
 * only guaranteed perfect if the original grid was a perfect maze.
 * (It's guaranteed perfect for the same reason the right-hand rule
 * will eventually take you back to the same spot in a perfect maze.)
 *
 * The process is take a maze and subdivide every cell in a new grid:
 *
 *      ______________.            ______________.
 * enter    |         |       enter__. | ______. |
 * enter    |______   |        exit  | |______ | |
 *      |   |         |            | | | ______| |
 *      |   |______   |            | | |______ | |
 *      |             |            | |_________| |
 *      |_____________|            |_____________|
 *
 * The difference between an entrance and a non-entrance is the need
 * to draw a line to the edge there.
 *
 * Returns NULL on error, or a new grid. The source grid is unmodified.
 */
GRID *
labyrinthgrid(GRID *g, int cid)
{
  GRID *lg;
  CELL *sc, *dc;
  int si,di,sj,dj,go,e,drows,dcols,nid;

  if(!g) { return NULL; }
  drows = g->rows * 2;
  dcols = g->cols * 2;
  lg = creategrid(drows, dcols, g->gtype);
  if(!lg) { return NULL; }

  /* si,sj is always source cell row,col
   * di,dj is always NW labyrinth cell; di+1 for S cell, dj+1 for E cell
   *          di-1 for neighbor to N, di+2 for neighbor to S
   *          dj-1 for neighbor to W, dj+2 for neighbor to E
   */
  for(si = 0; si < g->rows; si ++) {
    di = si * 2;
    for(sj = 0; sj < g->cols; sj ++) {
      dj = sj * 2;
      sc = visitrc(g, si, sj);
      if(!sc) { freegrid(lg); return NULL; }
      e = exitstatusbycell(sc);

      if(e == NO_EXITS) {
	/* probably masked off */
        continue;
      }

      /* NW corner of the new quadrant.
       * Connect N or W if original has N or W exits.
       * Connect S if original has no W exit.
       * Connect E if original has no N exit.
       */
      dc = visitrc(lg, di, dj);
      if(!dc) { freegrid(lg); return NULL; }
      dc->ctype = sc->ctype;
      if   (e & NORTH_EXIT) { dc->dir[NORTH] = (di - 1) * dcols + dj    ; }
      else                  { dc->dir[EAST]  = (di    ) * dcols + dj + 1; }
      if   (e & WEST_EXIT ) { dc->dir[WEST]  = (di    ) * dcols + dj - 1; }
      else                  { dc->dir[SOUTH] = (di + 1) * dcols + dj    ; }

      /* NE corner of the new quadrant.
       * Connect N or E if original has N or E exits.
       * Connect S if original has no E exit.
       * Connect W if original has no N exit.
       */
      dc = visitrc(lg, di, dj + 1);
      if(!dc) { freegrid(lg); return NULL; }
      dc->ctype = sc->ctype;
      if   (e & NORTH_EXIT) { dc->dir[NORTH] = (di - 1) * dcols + dj + 1; }
      else                  { dc->dir[WEST]  = (di    ) * dcols + dj    ; }
      if   (e & EAST_EXIT ) { dc->dir[EAST]  = (di    ) * dcols + dj + 2; }
      else                  { dc->dir[SOUTH] = (di + 1) * dcols + dj + 1; }

      /* SW corner of the new quadrant.
       * Connect S or W if original has S or W exits.
       * Connect N if original has no W exit.
       * Connect E if original has no S exit.
       */
      dc = visitrc(lg, di + 1, dj);
      if(!dc) { freegrid(lg); return NULL; }
      dc->ctype = sc->ctype;
      if   (e & SOUTH_EXIT) { dc->dir[SOUTH] = (di + 2) * dcols + dj    ; }
      else                  { dc->dir[EAST]  = (di + 1) * dcols + dj + 1; }
      if   (e & WEST_EXIT ) { dc->dir[WEST]  = (di + 1) * dcols + dj - 1; }
      else                  { dc->dir[NORTH] = (di    ) * dcols + dj    ; }

      /* SE corner of the new quadrant.
       * Connect S or E if original has S or E exits.
       * Connect N if original has no E exit.
       * Connect W if original has no S exit.
       */
      dc = visitrc(lg, di + 1, dj + 1);
      if(!dc) { freegrid(lg); return NULL; }
      dc->ctype = sc->ctype;
      if   (e & SOUTH_EXIT) { dc->dir[SOUTH] = (di + 2) * dcols + dj + 1; }
      else                  { dc->dir[WEST]  = (di + 1) * dcols + dj    ; }
      if   (e & EAST_EXIT ) { dc->dir[EAST]  = (di + 1) * dcols + dj + 2; }
      else                  { dc->dir[NORTH] = (di    ) * dcols + dj + 1; }

      if((cid != NC) && (sc->id == cid)) {
        /* okay, this is the entrance. Fix the off-grid directions */

        for(go = FIRSTDIR; go < FOURDIRECTIONS; go ++) {
	  nid = NC;

	  if(sc->dir[go] == cid) {
	    /* case one */
	    nid = 0;
	  }
	  else if((sc->dir[go] >= g->max) || (sc->dir[go] < NC)) {
	    /* cases two and three */
	    nid = sc->dir[go];
	  }

	  if(nid != NC) {
	    /* when 0, link back to self; otherwise link to original id */

            dc = visitrc(lg, (di + (go == SOUTH)?1:0), (dj + (go == EAST)?1:0) );
	    if(nid) { dc->dir[go] = nid; } else { dc->dir[go] = dc->id; }

            dc = visitrc(lg, (di + (go != NORTH)?1:0), (dj + (go != WEST)?1:0) );
	    if(nid) { dc->dir[go] = nid; } else { dc->dir[go] = dc->id; }

	  } /* found the direction to fix */
	}
      } /* if fixing an entrance */

    } /* for source column */
  } /* for source row */

  return lg;
} /* labyrinthgrid() */

/* This calculation ends up being needed a lot. If rotations happened
 * often, I'd make each case statement calculation into a macro.
 */
static
int
id_rotate( int rotation, int id, int rows, int cols )
{
  int i, j, ncols, ni, nj, nid;

  i = id / cols;
  j = id % cols;

  switch( rotation ) {
    case ROTATE_CW:
			ni = j;
			nj = rows - 1 - i;
			ncols = rows;
			break;

    case ROTATE_CCW:
    case ROTATE_CCW_ALT:
			ni = cols - 1 - j;
			nj = i;
			ncols = rows;
			break;

    case FLIP_TRANSPOSE:
			ni = j;
			nj = i;
			ncols = rows;
			break;

    case ROTATE_180:
			ni = rows - 1 - i;
			nj = cols - 1 - j;
			ncols = cols;
			break;

    case FLIP_LEFTRIGHT:
			ni = i;
			nj = cols - 1 - j;
			ncols = cols;
			break;

    case FLIP_TOPBOTTOM:
			ni = rows - 1 - i;
			nj = j;
			ncols = cols;
			break;

  } /* switch */
   
  nid = ni * ncols + nj;

  return nid;
} /* id_rotate() */


/* rotate a grid (CW, CCW, 180), flip a grid (TB or LR), or transpose
 * rows and columns
 *	ROTATE_CW	aka CLOCKWISE
 *	ROTATE_CCW	aka COUNTERCLOCKWISE	aka ROTATE_CCW_ALT
 *	ROTATE_180
 *	FLIP_TOPBOTTOM
 *	FLIP_LEFTRIGHT
 *	FLIP_TRANSPOSE	(sum of ROTATE_CCW and FLIP_LEFTRIGHT)
 *
 * returns 0 on success
 * on error, with no modification to the grid, returns NC
 */
int
rotategrid(GRID *g, int rotation)
{
  CELL *block;
  int oc, nc, nrows, ncols;
  if(!g) { return NC; }

  switch( rotation ) {
   case 0:              /* no-op */
		return 0;
		break;

    case ROTATE_CW:      /* fall through */
    case ROTATE_CCW:
    case ROTATE_CCW_ALT:
    case FLIP_TRANSPOSE:
    		nrows = g->cols;
		ncols = g->rows;
		break;
    
    case ROTATE_180:     /* fall through */
    case FLIP_LEFTRIGHT:
    case FLIP_TOPBOTTOM:
    		nrows = g->rows;
		ncols = g->cols;
		break;

    default: /* unknown rotation */
                return NC;
  } /* setup switch(rotation) */

  block = createcells(nrows, ncols, 0, 0);
  if(!block) {
    return NC;
  }

  for(oc = 0; oc < g->max; oc ++) {
    int oi, oj, ni, nj;
    int dirmap[DIRECTIONS];

    dirmap[UP]   = UP;
    dirmap[DOWN] = DOWN;
    CELL *src = &(g->cells[oc]);

    switch( rotation ) {
      case ROTATE_CW:
      				nc = id_rotate(rotation, oc, g->rows, g->cols);
				dirmap[NORTH] = EAST;
				dirmap[SOUTH] = WEST;
				dirmap[WEST]  = NORTH;
				dirmap[EAST]  = SOUTH;
				break ;

      case ROTATE_CCW:
      case ROTATE_CCW_ALT:
      				nc = id_rotate(rotation, oc, g->rows, g->cols);
				dirmap[NORTH] = WEST;
				dirmap[SOUTH] = EAST;
				dirmap[WEST]  = SOUTH;
				dirmap[EAST]  = NORTH;
				break ;

      case FLIP_TRANSPOSE:
      				nc = id_rotate(rotation, oc, g->rows, g->cols);
				dirmap[NORTH] = WEST;
				dirmap[SOUTH] = EAST;
				dirmap[WEST]  = NORTH;
				dirmap[EAST]  = SOUTH;
				break ;

      case ROTATE_180:
      				nc = id_rotate(rotation, oc, g->rows, g->cols);
				dirmap[NORTH] = SOUTH;
				dirmap[SOUTH] = NORTH;
				dirmap[WEST]  = EAST;
				dirmap[EAST]  = WEST;
				break ;

      case FLIP_LEFTRIGHT:
      				nc = id_rotate(rotation, oc, g->rows, g->cols);
				dirmap[NORTH] = NORTH;
				dirmap[SOUTH] = SOUTH;
				dirmap[WEST]  = EAST;
				dirmap[EAST]  = WEST;
				break ;

      case FLIP_TOPBOTTOM:
      				nc = id_rotate(rotation, oc, g->rows, g->cols);
				dirmap[NORTH] = SOUTH;
				dirmap[SOUTH] = NORTH;
				dirmap[WEST]  = WEST;
				dirmap[EAST]  = EAST;
				break ;
    } /* action switch(rotation) */
    
    CELL *dst = &(block[nc]);
    ni = nc / ncols;
    nj = nc % ncols;
    dst->id    = nc;
    dst->row   = ni;
    dst->col   = nj;
    dst->ctype = src->ctype;
    dst->name  = src->name;
    dst->data  = src->data;

    for(int go = FIRSTDIR; go < DIRECTIONS; go ++) {
      if(src->dir[go] == NC) { 
	dst->dir[dirmap[go]] = NC;
      } else {
	dst->dir[dirmap[go]] = id_rotate(rotation, src->dir[go],
      					g->rows, g->cols);
      }
    } /* loop over all directions */
  } /* loop over all cells */

  g->rows = nrows;
  g->cols = ncols;
  free(g->cells);
  g->cells = block;

  return 0;
} /* rotategrid() */

/* visit a cell by co-ordinates, return pointer or NULL */
#ifdef VISIT_FUNCTIONS
CELL*
visitrc(GRID *g, int i, int j)
{
  int index;

  if(!g || (i < 0) || (j < 0) || (i >= g->rows) || (j >= g->cols)) {
    return (CELL*)NULL;
  }

  index = (g->cols * i) + j;
  return &(g->cells[index]);
} /* visitrc() */

/* visit a cell directly by id, return pointer or NULL */
CELL*
visitid(GRID *g, int id)
{
  if(!g || (id < 0)) {
    return (CELL*)NULL;
  }

  if(id >= g->max) {
    return (CELL*)NULL;
  }

  return &(g->cells[id]);
} /* visitid() */
#else  /* VISIT_FUNCTIONS */
/* in grid.h */
#endif /* VISIT_FUNCTIONS */

/* from cell c,
 * visit a cell in direction d,
 * subject to connecton status cs:
 *   NC: only if not connected
 *   SYMMETRICAL: only if symmetrically connected
 *   THIS: only if connected in direction D
 *   ANY: not subject to connection status
 * return pointer or NULL
 */
CELL*
visitdir(GRID *g, CELL *c, int d, int cs)
{
  int i, j, ni, nj, id;

  if(!g || !c) {
    return (CELL*)NULL;
  }

  if( (d < FIRSTDIR) || (d > DIRECTIONS) ) {
    return (CELL*)NULL;
  }

  ni = i = c->row;
  nj = j = c->col;

  switch(d) {
    case NORTH: ni = i - 1; id = (g->cols * ni) + nj; break;
    case SOUTH: ni = i + 1; id = (g->cols * ni) + nj; break;
    case WEST:  nj = j - 1; id = (g->cols * ni) + nj; break;
    case EAST:  nj = j + 1; id = (g->cols * ni) + nj; break;
    case UP:   id = NC; break;	/* not yet implemented */
    case DOWN: id = NC; break;
    default:   id = NC;
  }

  /* off grid in any way? */
  if((id < 0) || (id > g->max) || (ni < 0) || (nj < 0) ||
     (ni >= g->rows) || (nj >= g->cols)) {
    return (CELL*)NULL;
  }

  /* now we know id is a possible cell, but does it meet the
   * connection status criteria?
   */

  CELL *that = &(g->cells[id]);

  /* don't care about connection */
  if(cs == ANY) {
    return that;
  }

  /* only if not connected */
  if(cs == NC) {
    if(c->dir[d] == NC) {
      return that;
    }
  }

  /* only if c in dir d connects to that */
  if(cs == THIS) {
    if(c->dir[d] == id) {
      return that;
    }
  }

  /* only if c in dir d connects to that, and
   * that connects to c in opposite direction */
  if(cs == SYMMETRICAL) {
    int od = opposite(d);
    if((c->dir[d] == id) && (c->id == that->dir[od])) {
      return that;
    }
  }

  return (CELL*)NULL;
} /* visitdir() */

/* a random cell in the grid */
CELL *
visitrandom(GRID *g)
{
  if(!g) { return (CELL*)NULL; }
  
  return visitid(g, random() % g->max);
}

/* {FOO}bycell functions use one or two CELL pointers
 * {FOO}byrc functions take GRID and one or two pairs of row,col
 * {FOO}byid functions take GRID and one or two ids
 *
 * connect{BAR} create a connection
 * isconnected{BAR} test a connection
 * disconnect{BAR} remove a connection
 * natdirection{BAR} returns the expected direction from C1 to C2
 */

/* connection creation */
/* c2 to c1 direction (c1c2d) need not be opposite of
 * c1 to c2 (c2c1d), and either can be NC: no connection.
 * use SYMMETRICAL for c2c1d to automatically use opposite dir
 */
void 
connectbycell(CELL *c1, int c1c2d, CELL * c2, int c2c1d)
{
  if(!c1) { return; }
  if(!c2) { return; }

  if(c2c1d == SYMMETRICAL) { c2c1d = opposite(c1c2d); }

  if(c1c2d > DIRECTIONS) { return; }
  if(c2c1d > DIRECTIONS) { return; }
  if(c1c2d < NC) { return; }
  if(c2c1d < NC) { return; }

  if(c1c2d > NC) {
    c1->dir[c1c2d] = c2->id;
  }
  if(c2c1d > NC) {
    c2->dir[c2c1d] = c1->id;
  }
} /* connectbycell() */

void connectbyrc(GRID *g, int r1, int c1, int c1c2d,
                          int r2, int c2, int c2c1d)
{
  if(!g) { return; }
  connectbycell(visitrc(g, r1, c1), c1c2d,
                visitrc(g, r2, c2), c2c1d);
} /* connectbyrc() */

void connectbyid(GRID *g, int id1, int c1c2d,
                          int id2, int c2c1d)
{
  if(!g) { return; }
  connectbycell(visitid(g, id1), c1c2d,
                visitid(g, id2), c2c1d);
} /* connectbyid() */


/* connection removal */
/* c2 to c1 direction (c1c2d) need not be opposite of
 * c1 to c2 (c2c1d), and either can be NC: no connection.
 *    during disconnect, NC means don't try to remove a connection
 * use SYMMETRICAL for c2c1d to automatically use opposite dir
 */
void 
disconnectbycell(CELL *c1, int c1c2d, CELL * c2, int c2c1d)
{
  if(!c1) { return; }
  if(!c2) { return; }

  if(c2c1d == SYMMETRICAL) { c2c1d = opposite(c1c2d); }

  if(c1c2d > DIRECTIONS) { return; }
  if(c2c1d > DIRECTIONS) { return; }
  if(c1c2d < NC) { return; }
  if(c2c1d < NC) { return; }

  if(c1c2d > NC) {
    c1->dir[c1c2d] = NC;
  }
  if(c2c1d > NC) {
    c2->dir[c2c1d] = NC;
  }
} /* disconnectbycell() */

void
disconnectbyrc(GRID *g, int r1, int c1, int c1c2d,
                        int r2, int c2, int c2c1d)
{
  if(!g) { return; }
  disconnectbycell(visitrc(g, r1, c1), c1c2d,
                   visitrc(g, r2, c2), c2c1d);
} /* disconnectbyrc() */

void
disconnectbyid(GRID *g, int id1, int c1c2d,
                        int id2, int c2c1d)
{
  if(!g) { return; }
  disconnectbycell(visitid(g, id1), c1c2d,
                   visitid(g, id2), c2c1d);
} /* disconnectbyid() */



/* check connections */

/* for two cells, returns direction by which c1 connects to c2
 * or NC if no connect.
 * if d is ANYDIR, will try all directions, and returning first found,
 * but if d is a particular direction will only try that one.
 * NOTE that zero is a valid direction, so test results against NC
 * or an expected direction, eg (isconnectedbycell(a, b, dir) == dir)
 */
int 
isconnectedbycell(CELL *c1, CELL *c2, int d)
{
  if(!c1) { return NC; }
  if(!c2) { return NC; }

  if(d != ANYDIR) {
    if(d > DIRECTIONS) { return NC; }
    if(d < FIRSTDIR) { return NC; }

    if(c1->dir[d] == c2->id) {
      return d;
    }
    return NC;
  }

  for(d = FIRSTDIR; d < DIRECTIONS; d ++) {
    if(c1->dir[d] == c2->id) {
      return d;
    }
  }
  return NC;
} /* isconnectedbycell() */

int 
isconnectedbyrc(GRID *g, int r1, int c1, int r2, int c2, int d)
{
  if(!g) { return NC; }

  return isconnectedbycell(visitrc(g,r1,c1), visitrc(g,r2,c2), d);
} /* isconnectedbyrc() */

int 
isconnectedbyid(GRID *g, int id1, int id2, int d)
{
  if(!g) { return NC; }

  return isconnectedbycell(visitid(g,id1), visitid(g,id2), d);
} /* isconnectedbyid() */


/* for a particular cell, return information about the edges it
 * adjoins. Edges are returned as bit status flags, so check
 * results accordingly.
 */
int
edgestatusbycell(GRID *g, CELL *c)
{
  int edges = 0;
  if(!g) { return EDGE_ERROR; }
  if(!c) { return EDGE_ERROR; }

  if(c->row == 0) {           edges = edges|NORTH_EDGE; }
  if(c->col == 0) {           edges = edges|WEST_EDGE; }
  if(c->row == g->rows - 1) { edges = edges|SOUTH_EDGE; }
  if(c->col == g->cols - 1) { edges = edges|EAST_EDGE; }

  if (edges == 0) {           edges = NO_EDGES; }
  return edges;
} /* edgestatusbycell() */

int
edgestatusbyrc(GRID *g, int i, int j)
{
  if(!g) { return EDGE_ERROR; }

  return edgestatusbycell(g, visitrc(g,i,j));
} /* edgestatusbyrc */

int
edgestatusbyid(GRID *g, int id)
{
  if(!g) { return EDGE_ERROR; }

  return edgestatusbycell(g, visitid(g,id));
} /* edgestatusbyid */

/* Neighbor counting.
 * There are three ways to count neighbors:
 *   1. Any neighbors at all (concern==NEIGHBORS), which will be 2
 *      on a typical corner and 4 on a typical interior cell.
 *   2. Any neighbors we can exit to (concern==EXITS), which will be
 *      1 on a dead end, 0 on all cells of a brand new grid, and 4
 *      at most.
 *   3. Any neighbors with a particular ctype (concern==OF_TYPE).
 *      This is the only counting method that uses the t value.
 * Returns a negative value on error.
 */
int
ncountbycell(GRID *g, CELL *c, int concern, int t)
{
  int count = 0;
  if(!g) { return NC; }
  
  for (int d = FIRSTDIR; d < FOURDIRECTIONS; d++) {
    if(concern==EXITS) {
      if(c->dir[d] != NC) { count ++; }
    } else {
      /* not interested in exits */
      CELL *nc = visitdir(g, c, d, ANY);
      
      if(nc) {
	switch(concern) {
	  case NEIGHBORS: count ++;
			  break;
	  case OF_TYPE  : if(nc->ctype == t) { count ++; }
			  break;
	  default: /* bad concern */
		   return -2;
	}
      }
    } /* concern NOT exits */
  } /* for direction */

  return count;
} /* ncountbycell() */

int
ncountbyrc(GRID *g, int r, int c, int concern, int t)
{
  if(!g) { return NC; }

  return ncountbycell(g, visitrc(g,r,c), concern, t);
} /* ncountbyrc() */

int
ncountbyid(GRID *g, int id, int concern, int t)
{
  if(!g) { return NC; }

  return ncountbycell(g, visitid(g,id), concern, t);
} /* ncountbyid() */



/* for a particular cell, return information about the walls it
 * adjoins. Walls are returned as bit status flags, so check
 * results accordingly.
 */
int
wallstatusbycell(CELL *c)
{
  int walls = 0;
  if(!c) { return WALL_ERROR; }

  if(c->dir[NORTH] == NC) { walls = walls|NORTH_WALL; }
  if(c->dir[WEST]  == NC) { walls = walls|WEST_WALL; }
  if(c->dir[SOUTH] == NC) { walls = walls|SOUTH_WALL; }
  if(c->dir[EAST]  == NC) { walls = walls|EAST_WALL; }

  if (walls == 0) { walls = NO_WALLS; }
  return walls;
} /* wallstatusbycell() */

int
wallstatusbyrc(GRID *g, int i, int j)
{
  if(!g) { return WALL_ERROR; }

  return wallstatusbycell(visitrc(g,i,j));
} /* wallstatusbyrc() */

int
wallstatusbyid(GRID *g, int id)
{
  if(!g) { return WALL_ERROR; }

  return wallstatusbycell(visitid(g,id));
} /* wallstatusbyid() */


/* for a particular cell, return information about the exits
 * from it. Exits are returned as bit status flags, so check
 * results accordingly. Unlike walls or edges, this does UP/DOWN.
 */
int
exitstatusbycell(CELL *c)
{
  int exits = 0;
  if(!c) { return EXIT_ERROR; }

  if(c->dir[NORTH] != NC) { exits = exits|NORTH_EXIT; }
  if(c->dir[WEST]  != NC) { exits = exits|WEST_EXIT; }
  if(c->dir[SOUTH] != NC) { exits = exits|SOUTH_EXIT; }
  if(c->dir[EAST]  != NC) { exits = exits|EAST_EXIT; }
  if(c->dir[UP]    != NC) { exits = exits|UP_EXIT; }
  if(c->dir[DOWN]  != NC) { exits = exits|DOWN_EXIT; }

  if (exits == 0) { exits = NO_EXITS; }
  return exits;
} /* exitstatusbycell() */

int
exitstatusbyrc(GRID *g, int i, int j)
{
  if(!g) { return EXIT_ERROR; }

  return exitstatusbycell(visitrc(g,i,j));
} /* exitstatusbyrc() */

int
exitstatusbyid(GRID *g, int id)
{
  if(!g) { return EXIT_ERROR; }

  return exitstatusbycell(visitid(g,id));
} /* exitstatusbyid() */

/* 
 * natdirection{BAR} returns the expected direction from C1 to C2
 *			one of NORTH, SOUTH, EAST, WEST if adjacent
 *			returns SYMMETRICAL if C1 is C2
 *			returns NC if not adjacent or the same cell
 */
int
natdirectionbycell(CELL *c1, CELL *c2)
{
  if(!c1) { return NC; }
  if(!c2) { return NC; }
  if((c1 == c2) || (c1->id == c2->id)) { return SYMMETRICAL; }
  
  if( c1->col == c2->col ) {
    if ( c1->row == c2->row+1 ) { return NORTH; }
    if ( c1->row == c2->row-1 ) { return SOUTH; }
  }
  else if( c1->row == c2->row ) {
    if ( c1->col == c2->col+1 ) { return WEST; }
    if ( c1->col == c2->col-1 ) { return EAST; }
  }
  return NC;

} /* natdirectionbycell() */

int
natdirectionbyrc(GRID *g, int i1, int j1, int i2, int j2)
{
  if(!g) { return NC; }

  return natdirectionbycell(visitrc(g,i1,j1), visitrc(g,i2,j2));
} /* natdirectionbyrc() */

int
natdirectionbyid(GRID *g, int id1, int id2)
{
  if(!g) { return WALL_ERROR; }

  return natdirectionbycell(visitid(g,id1), visitid(g,id2));
} /* natdirectionbyid() */


/* mallocs space and copies a name to a cell */
/* returns -3 if no cell,
 * -2 if malloc failed after free()ing old name
 * -1 if malloc failed with no old name
 * 0 if no previous name
 * 1 if a previous name had to be free()d
 */
int
namebycell(CELL *c, char *name)
{
  int rc = 0;
  int len;
  if(!c) { return -3; }
  if(c->name) { free(c->name); rc = 1; }
  len = strnlen(name, BUFSIZ - 1) + 1;
  c->name = (char*)malloc((size_t)len);
  if(!c->name) { return -1 - rc; }
  strncpy(c->name, name, len);
  return rc;
} /* namecell() */

int
namebyrc(GRID *g, int i, int j, char *name)
{
  if(!g) { return -3; }
  return namebycell(visitrc(g,i,j), name);
} /* namebyrc() */

int
namebyid(GRID *g, int id, char *name)
{
  if(!g) { return -3; }
  return namebycell(visitid(g,id), name);
} /* namebyid() */


/* grid equivilent of namebycell() */
int
namegrid(GRID *g, char*name)
{
  int rc = 0;
  int len;
  if(!g) { return -3; }
  if(g->name) { free(g->name); rc = 1; }
  len = strnlen(name, BUFSIZ - 1) + 1;
  g->name = (char*)malloc((size_t)len);
  if(!g->name) { return -1 - rc; }
  strncpy(g->name, name, len);
  return rc;
} /* namegrid() */


/* for every cell in grid, run provided function and
 * return sum of results
 */
int
iterategrid(GRID *g, int (*ifunc)(GRID *,CELL *,void *), void *param)
{
  int rc = 0;
  int i, j;

  for(i = 0; i < g->rows; i ++) {
    for(j = 0; j < g->cols; j ++) {
      rc += ifunc(g, visitrc(g,i,j), param);
    }
  }
  return rc;
} /* iterategrid() */

/* for every cell in row i of grid, run provided function and
 * return sum of results
 */
int
iteraterow(GRID *g, int i, int (*ifunc)(GRID *,CELL *,void *), void *param)
{
  int rc = 0;
  if(!g || (i < 0) || (i >= g->rows)) {
    return -1;
  }

  for(int j = 0; j < g->cols; j++) {
    rc += ifunc(g, visitrc(g,i,j), param);
  }
  return rc;
} /* iteraterow() */

int
iteratecol(GRID *g, int j, int (*ifunc)(GRID *,CELL *,void *), void *param)
{
  int rc = 0;
  if(!g || (j < 0) || (j >= g->cols)) {
    return -1;
  }

  for(int i = 0; i < g->rows; i++) {
    rc += ifunc(g, visitrc(g,i,j), param);
  }
  return rc;
} /* iteratecol() */


/* return the opposite of a direction */
int
opposite(int d)
{
  switch(d) {
    case NORTH: return SOUTH;
    case SOUTH: return NORTH;
    case WEST: return EAST;
    case EAST: return WEST;
    case UP: return DOWN;
    case DOWN: return UP;
    default: return NC;
  }
} /* opposite() */

/* turn a direction into a string (eg for debugging) */
const char *
dirtoname(int d)
{
   struct ns { char *name; };
   const struct ns dirnames[] = {
     { "north" },
     { "west" },
     { "east" },
     { "south" },
     { "up" },
     { "down" },
     { "not a direction" },
   };

   if((d < 0) || (d > DIRECTIONS)) { d = DIRECTIONS; }
   return dirnames[d].name;
} /* dirtoname() */


/* create a ASCII art version of the grid,
 * use_name is a boolean, if true print names in cells
 * (but will be clipped to 3 chars)
 */
char *
ascii_grid(GRID *g, int use_name)
{
  char *out = NULL;
  int i, j, p, outsize;
  char left, top;
  CELL *here, *west, *north;
  
  if(!g) { return out; }

  p = 0; /* index into out */

  /* a cell is "+---" (4) wide + "+\n" (2) on last cell
   *                   |     cols are "-" (2) tall + "-" (1) on last row
   *                   |              " "  |
   *                   |                   |           + final
   *                   |                   |           | null
   *                   v                   v           v    */
  outsize = (g->cols * 4 + 2) * (g->rows * 2 + 1) +    1 ;
  if(use_name && g->name) {
    p = 1 + strnlen(g->name, BUFSIZ);
    use_name += p;
  }
  out = (char*) malloc(outsize);
  if(!out) { return out; }

  if(use_name && g->name) {
    strncpy(out, g->name, p);
    /* replace the final null */
    out[p-1] = '\n';
  }
    
  for(i = 0; i < g->rows; i ++) { /* grid row */
    for(int l = 0; l < 2; l ++) { /* line of output per row */
      for(j = 0; j < g->cols; j ++) { /* grid col */
        left = '|';
        top = '-';
	here = visitrc(g,i,j);

        if(i) {
	  north = visitrc(g,i-1,j);
	  if(isconnectedbycell(here, north, NORTH) != NC) {
	    top = ' ';
	  } else {
            top = '-';
	  }
	}

        if(j) {
	  west = visitrc(g,i,j-1);
	  if(isconnectedbycell(west, here, EAST) != NC) {
	    left = ' ';
	  } else {
	    left = '|';
	  }
	}

        if(l) {
	  out[p++] =left;
	  if(use_name && here->name) {
	    char *s = here->name;
	    if(*s) { out[p++] = *s; s++; } else { out[p++] = ' '; }
	    if(*s) { out[p++] = *s; s++; } else { out[p++] = ' '; }
	    if(*s) { out[p++] = *s; s++; } else { out[p++] = ' '; }
	  } else {
	    out[p++] = ' '; out[p++] = ' '; out[p++] = ' ';
	  }
	} else {
	  out[p++] = '+'; out[p++] = top; out[p++] = top; out[p++] = top;
	}
      }
      /* end of line */
      if(l) { 
        out[p++] = '|';
      } else {
        out[p++] = '+';
      }
      out[p++] = '\n';
    }
  }
  /* final line */
  for(j = 0; j < g->cols; j ++) { /* grid col */
    out[p++] = '+'; out[p++] = '-'; out[p++] = '-'; out[p++] = '-';
  }
  out[p++] = '+';
  out[p++] = '\n';
  out[p++] = '\0';

  return out;
} /* ascii_grid() */

