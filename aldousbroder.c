/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */

/* Third maze from Mazes for Programmers,
 * this is a random walk maze that produces good results
 * but is slow to generate.
 *
 * Example maze:
 *
 * +---+---+---+---+---+---+---+---+---+---+
 * |                           |           |
 * +---+---+---+   +---+   +   +---+---+   +
 * |       |       |   |   |   |   |       |
 * +---+   +---+---+   +---+   +   +---+   +
 * |                       |       |   |   |
 * +   +---+---+---+---+   +   +   +   +   +
 * |       |           |       |           |
 * +---+   +---+---+   +   +---+---+---+   +
 * |       |               |   |           |
 * +   +---+   +---+   +---+   +---+   +---+
 * |   |       |               |   |   |   |
 * +   +   +   +---+---+---+   +   +   +   +
 * |   |   |           |       |       |   |
 * +   +---+   +   +---+---+   +---+---+   +
 * |   |       |       |                   |
 * +---+---+   +   +   +---+---+---+   +---+
 * |       |   |   |               |   |   |
 * +   +   +   +---+---+---+---+   +---+   +
 * |   |               |                   |
 * +---+---+---+---+---+---+---+---+---+---+
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "grid.h"
#include "distance.h"

#define UNVISITED	1
#define VISITED		2
#define NEEDDIR		(99+DIRECTIONS)


/* Named for David Aldous and Andrei Broder, this method cannot
 * use the grid iterator because it needs to visit cells randomly,
 * and it needs to be able to revisit cells.
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

int
main(int notused, char**ignored)
{
  GRID *g;
  DMAP *dm;
  char *board;
  int rc;

  g = creategrid(10,10,UNVISITED);
  rc = aldbro(g);
  if(rc) {
    printf("Um, issue.\n");
  }

  printf("Unsolved\n");
  board = ascii_grid(g, 0);
  puts(board);
  free(board);

  dm = findlongestpath(g);
  if(!dm) {
    printf("Ooops, solver broke\n");
  }
  namepath(dm, " A", " x", " B");

  printf("\nSolved (for longest possible path)\n");
  board = ascii_grid(g, 1);
  puts(board);
  free(board);

  freegrid(g);
  return 0;
}
