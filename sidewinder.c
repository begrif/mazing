/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */

/* Second maze from Mazes for Programmers,
 * these are a bit more interesting.
 *
 * Example maze:
 *
 * +---+---+---+---+---+---+---+---+---+---+
 * |                                       |
 * +---+   +---+---+---+---+---+   +   +   +
 * |   |   |                       |   |   |
 * +   +---+   +   +---+---+---+   +---+   +
 * |   |       |       |           |       |
 * +   +---+---+   +   +   +---+   +   +   +
 * |           |   |   |   |       |   |   |
 * +   +   +   +   +   +---+---+---+   +   +
 * |   |   |   |   |       |           |   |
 * +   +---+   +   +---+   +   +---+   +---+
 * |       |   |       |   |       |       |
 * +   +---+---+   +   +   +   +   +   +   +
 * |           |   |   |   |   |   |   |   |
 * +   +   +---+   +---+   +   +   +---+   +
 * |   |       |   |       |   |       |   |
 * +---+   +   +---+---+---+   +   +   +---+
 * |       |               |   |   |       |
 * +   +---+---+---+   +---+---+---+---+   +
 * |       |                       |       |
 * +---+---+---+---+---+---+---+---+---+---+
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "grid.h"

typedef struct {
  int runlength;
  int runstart_r;
  int runstart_c;
} tree_status;

int
treewalker(GRID *g, CELL *c, void* status)
{
  tree_status *ts;
  CELL *cc, *oc;
  int edges;
  int closeit;
  int go;
  int nc;

  if(!g) { return -1; }
  if(!c) { return -1; }

  ts = (tree_status*)status;

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
}

int
main(int notused, char**ignored)
{
  GRID *g;
  char *board;
  tree_status stats;

  stats.runlength = 0;

  g = creategrid(10,10,1);
  iterategrid(g, treewalker, &stats);

  board = ascii_grid(g, 0);
  puts(board);

  free(board);
  freegrid(g);
  return 0;
}
