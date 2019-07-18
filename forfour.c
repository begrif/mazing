/* July 2019, Benjamin Elijah Griffin / Eli the Bearded */

/* More generally usable bits of code written for four */

#include <stdio.h> /* one error message in colors() */
#include <stdlib.h> /* for malloc() at least */

#include "forfour.h"

/* an interatewalk callback to set cell type */
int
marksolved(DMAP *dm, int cid, void *unused)
{
  CELL *c;

  c = visitid(dm->grid, cid);
  if(c) {
    c->ctype = SOLVEDCELL;
    return 1;
  }
  return 0;
} /* marksolved() */


/* Passed in a string, returns 1 if a valid color code and 0 otherwise.
 * Valid colors are six bytes of case-insensitive hexadecimal or three
 * bytes of lower case a to z ("Prof Fir colors")
 */
int
verifycolor(char *color)
{
  char *p;
  int count = 0, mode = 0;

  /* mode 0: unknown case.
   * mode 1: hexadecimal
   * mode 2: Prof Fir
   */

  if(!color) { return 0; }
  p = color;
  if(!*p) { return 0; }

  while(*p) {
    if(                count > 6 ) { return 0; }
    if((mode == 2) && (count > 3)) { return 0; }
    if((mode == 0) && (count > 3)) { mode = 1; }

    if( *p < '0' )               { return 0; }
    if((*p > '9') && (*p < 'A')) { return 0; }
    if((*p > 'F') && (*p < 'a')) { return 0; }
    if( *p > 'z' )               { return 0; }

    if( *p > 'f' ) { if(mode == 1) { return 0; } else { mode = 2; } }
    if( *p < 'A' ) { if(mode == 2) { return 0; } else { mode = 1; } }

    p++;
    count ++;
  }
  if((mode == 2) && (count != 3)) { return 0; }
  if((mode == 1) && (count != 6)) { return 0; }

  return 1;
} /* verifycolor() */


/* Dead simple turn two letters of hexadecimal into one 8 bit value.
 * Does no error-checking ("dead simple").
 */
int
hexpair(char a, char b)
{
  int n = 0;
  if        (a < 'A') { n  =      a - '0'; }
    else if (a < 'a') { n  = 10 + a - 'A'; }
    else              { n  = 10 + a - 'a'; }
  n <<= 4;
  if        (b < 'A') { n +=      b - '0'; }
    else if (b < 'a') { n += 10 + b - 'A'; }
    else              { n += 10 + b - 'a'; }
  return n;
} /* hexpair() */


/* Dead simple turn one letter into one 8 bit value. This is based on
 * "Prof Fir" (comp.lang.c persona) color words. The idea is a..z is a
 * good approximation for intervals in 0 to 255 and three letter words
 * are a compact way to convey a color. Does no error checking.
 */
int
proffir(char a)
{
  if(a == 'a') { return 0; }
  if(a == 'z') { return 255; }

  /* 25500 / 26 = 980, so 9.8 is a good step size. Do the math at 10x
   * to make it integer friendly.
   */
  return( (98 * (a - 'a')) / 10 );
} /* proffir() */


/* Copy a string of 3 or 6 characters and a flag character
 * into the next empty slot of an array of short strings.
 * The colors are denoted by a flag in char 7 of the array.
 * The optarg string is presumed to have been previously
 * syntax checked with verifycolor().
 */
void
copycolor(color_overide_t usercolors, char flag, char *optarg)
{
  int n = 0;
  char *u;
  
  do {
    u = usercolors[n];
    n++;
  } while(*u);
  
  n = 0;
  u[n] = optarg[n]; n++;
  u[n] = optarg[n]; n++;
  u[n] = optarg[n]; n++;
  if(optarg[n]) {
    /* 6 char color */
    u[n] = optarg[n]; n++;
    u[n] = optarg[n]; n++;
    u[n] = optarg[n]; n++;
  } else {
    /* 3 char color */
    u[n] = 0; n++;
    u[n] = 0; n++;
    u[n] = 0; n++;
  }
  u[n] = flag; n++;
  u[n] = 0;
} /* copycolor() */


/* create a colordata structure */
COLORDATA *
colors(color_overide_t usercolors)
{
  COLORDATA *cd;

  cd = malloc(sizeof(COLORDATA));
  if(!cd) { return NULL; }

  *cd = (COLORDATA) {
     .wall = {   0,   0,   0, 0 },	/* interior walls */
     .edge = {   0,   0,   0, 0 },	/* exterior walls */
     .uc1  = { 240, 240,   0, 0 },	/* first cell background */
     .uc2  = { 240,   0, 240, 0 },	/* and last cell background */
     .fg   = {   0, 210,   0, 0 },	/* line for solved path */
     .bg   = { 250, 250, 250, 0 },	/* most cells background */
  };

  if(usercolors && usercolors[0] && usercolors[0][0]) {
    int n, ishex, *carray;
    char *p;

    /* All colors are pre-vetted for being 6 characters of hexadecimal
     * or 3 characters of lowercase letters, so "byte 4 is a null" will
     * reliably tell them apart. Byte 7 is which color and byte 8 is
     * another null.
     */
    n = 0;
    p = usercolors[n];
    while( *p ) {
      ishex = p[3]; /* fourth character */
      switch( p[6] ) {
	 /* { "edge",      required_argument,  0,  'E' }, */
	 case 'E': carray = (cd->edge); break;

	 /* { "wall",      required_argument,  0,  'W' }, */
	 case 'W': carray = (cd->wall); break;

	 /* { "start",     required_argument,  0,  'S' }, */
	 case 'S': carray = (cd->uc1); break;

	 /* { "finish",    required_argument,  0,  'F' }, */
	 case 'F': carray = (cd->uc2); break;

	 /* { "bg",        required_argument,  0,  'B' }, */
	 case 'B': carray = (cd->bg); break;

	 /* { "answer",    required_argument,  0,  'A' }, */
	 case 'A': carray = (cd->fg); break;

	 default:
	   fprintf(stderr, "Color setting function broken.\n");
	   return NULL;
      } /* switch */

      if(ishex) {
        carray[0] = hexpair(p[0], p[1]);
        carray[1] = hexpair(p[2], p[3]);
        carray[2] = hexpair(p[4], p[5]);
      } else {
        carray[0] = proffir(p[0]);
        carray[1] = proffir(p[1]);
        carray[2] = proffir(p[2]);
      }
      n ++;
      p = usercolors[n];
    } /* for entry in usercolors */
  } /* user color overrides */
  return cd;
} /* colors() */


/* Draws cells with double-wide walls, normally.
 * Eg, a top entry dead end 7x7 will have walls on left and right,
 * not just rely on the wall from the cell to the other side. At
 * very small sizes (< 5), this changes to just do one side. Edges
 * will still draw, however, so the edge cells will be narrow.
 *
 *   one        several side by side
 *   cell
 * w     w      w     ww     ww     e
 * w     w      w     ww     ww     e
 * w     w      w     ww     ww     e
 * w     w      w     ww     ww     e	7x7
 * w     w      w     ww     ww     e
 * w     w      w     ww     ww     e
 * wwwwwww      wwwwwwwwwwwwwwwwwwwwe
 *
 *   w          w   w   w  e
 *   w          w   w  	w  e		4x4
 *   w          w   w   w  e
 *   wwww       wwwwwwwwwwwe
 *
 * And if the grid type and cell type are both SOLVEDCELL, then
 * also draw the answer path in.
 *
 * A drawmaze() callback, a bit more complicated with the solution
 * code than default_drawcell() in mazeimgs.c, but also a bit
 * simpler in only supporting RGB color.
 */
int
cleandraw(MAZEBITMAP *mb, png_byte *image, CELL *c)
{
  int walls, edges, i1, i2, j1, j2;
  int wid, hei, pix, chn, two;
  int tol, mid, full;
  int allexits, pathexit[4];
  COLORDATA *colors;

  if((!mb) || (!mb->dmap) || (!mb->dmap->grid) || (!mb->udata)) {
    return 0;
  }
  colors = (COLORDATA *)mb->udata;
  wid = mb->cell_w;
  hei = mb->cell_h;
  chn = mb->channels;
  two = mb->doubled + 1;
  pix = wid * hei;

  tol = 0; /* top or left */
  full = wid - 1;
  mid = wid / 2;
  if (wid == 3) { mid = 1; }

  /* If the maze entrance / exit moves, this needs to be fixed. */
  if(c->id == 0) {
    fill_cell(image, pix, chn, two, &(colors->uc1[0]));
    /* force a check of all neighboring cells */
    allexits = 8;
  } else if(c->id == (mb->dmap->grid->max - 1)) {
    fill_cell(image, pix, chn, two, &(colors->uc2[0]));
    /* force a check of all neighboring cells */
    allexits = 8;
  } else {
    fill_cell(image, pix, chn, two, &(colors->bg[0]));
    allexits = 0;
  }

  walls = wallstatusbycell(c);
  edges = edgestatusbycell(mb->dmap->grid, c);

  /* North / South edges also have the exit. Don't draw that edge if
   * an exit cell.
   */
  if((edges & NORTH_EDGE) || (walls & NORTH_WALL)) {
    i1 = i2 = tol;
    j1 = tol;
    j2 = full;

    if((edges & NORTH_EDGE) && (walls & NORTH_WALL)) {
      pathexit[NORTH] = NC;
      draw_a_line(image, i1, j1, i2, j2, wid, chn, two, &(colors->edge[0]));
    } else {
      /* no north wall at small size */
      if(walls & NORTH_WALL) {
        pathexit[NORTH] = NC; 
	if (wid > 4) {
	  draw_a_line(image, i1, j1, i2, j2, wid, chn, two, &(colors->wall[0]));
	}
      }
    }
  } else {
    pathexit[NORTH] = MAYBE;
    allexits ++;
  }

  if((edges & SOUTH_EDGE) || (walls & SOUTH_WALL)) {
    i1 = i2 = full;
    j1 = tol;
    j2 = full;

    if((edges & SOUTH_EDGE) && (walls & SOUTH_WALL)) {
      pathexit[SOUTH] = NC;
      draw_a_line(image, i1, j1, i2, j2, wid, chn, two, &(colors->edge[0]));
    } else {
      if(walls & SOUTH_WALL) {
        pathexit[SOUTH] = NC;
        draw_a_line(image, i1, j1, i2, j2, wid, chn, two, &(colors->wall[0]));
      }
    }
  } else {
    pathexit[SOUTH] = MAYBE;
    allexits ++;
  }


  /* East / West are easier, no edge exits to worry about. */
  if((edges & EAST_EDGE) || (walls & EAST_WALL)) {
    j1 = j2 = full;
    i1 = tol;
    i2 = full;
    /* adjust to not overwrite an edge */
    if(edges & NORTH_EDGE) { i1 ++; }
    if(edges & SOUTH_EDGE) { i2 --; }
    pathexit[EAST] = NC;

    /* no east wall at small size but still need edges */
    if(edges & EAST_EDGE) {
      draw_a_line(image, i1, j1, i2, j2, wid, chn, two, &(colors->edge[0]));
    }
    else if((walls & EAST_WALL) && (wid > 4)) {
      draw_a_line(image, i1, j1, i2, j2, wid, chn, two, &(colors->wall[0]));
    }
  } else {
    pathexit[EAST] = MAYBE;
    allexits ++;
  }

  if((edges & WEST_EDGE) || (walls & WEST_WALL)) {
    j1 = j2 = tol;
    i1 = tol;
    i2 = full;
    /* adjust to not overwrite an edge */
    if(edges & NORTH_EDGE) { i1 ++; }
    if(edges & SOUTH_EDGE) { i2 --; }
    pathexit[WEST] = NC;

    if(edges & WEST_EDGE) {
      draw_a_line(image, i1, j1, i2, j2, wid, chn, two, &(colors->edge[0]));
    } else {
      draw_a_line(image, i1, j1, i2, j2, wid, chn, two, &(colors->wall[0]));
    }
  } else {
    pathexit[WEST] = MAYBE;
    allexits ++;
  }

  if((c->ctype == SOLVEDCELL) && (mb->dmap->grid->gtype == SOLVEDCELL)) {

    /* always draw two line segments, exit to center, even if it could
     * be a straight line between two exits.
     */
    for(int go = FIRSTDIR; go < FOURDIRECTIONS; go++) {

      /* If more than two possible exits, figure out which ones to discard
       * On edge exits, allexits is forced to > 4, and we don't discard
       * the visit errors.
       */
      if(allexits > 2) {
	if(MAYBE == pathexit[go]) {
	  /* off-grid entrance and exit directions are not symmetrical
	   * only check that THIS direction works.
	   */
	  CELL *overthere = visitdir(mb->dmap->grid, c, go, THIS);
	  if(overthere && (overthere->ctype != SOLVEDCELL)) {
	    pathexit[go] = NC;
	    allexits --;
	  }
	}
      } /* more than two exits */

      if(pathexit[go] != NC)  {
	switch(go) {
	  case NORTH: i1 = tol; i2 = mid;  j1 = mid; j2 = mid;  break;
	  case SOUTH: i1 = mid; i2 = full; j1 = mid; j2 = mid;  break;
	  case WEST:  i1 = mid; i2 = mid;  j1 = tol; j2 = mid;  break;
	  case EAST:  i1 = mid; i2 = mid;  j1 = mid; j2 = full; break;
	}
	draw_a_line(image, i1, j1, i2, j2, wid, chn, two, &(colors->fg[0]));
      } /* if an exit */
    } /* for each direction */

  } /* if drawing a solution in this cell */

  return 1;
} /* cleandraw(); */

