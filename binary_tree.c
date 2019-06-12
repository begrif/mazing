/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */

/* Simplest maze from Mazes for Programmers,
 * basically only useful for testing the code.
 *
 * Example maze:
 *
 * +---+---+---+---+---+---+---+---+---+---+
 * |                                       |
 * +---+---+   +   +---+   +---+   +---+   +
 * |           |   |       |       |       |
 * +---+   +   +---+   +   +   +---+   +   +
 * |       |   |       |   |   |       |   |
 * +   +---+---+   +---+   +---+   +---+   +
 * |   |           |       |       |       |
 * +   +---+   +---+   +---+---+   +---+   +
 * |   |       |       |           |       |
 * +---+---+---+   +   +   +   +---+   +   +
 * |               |   |   |   |       |   |
 * +   +---+   +   +---+---+   +   +---+   +
 * |   |       |   |           |   |       |
 * +   +---+---+   +---+   +   +   +   +   +
 * |   |           |       |   |   |   |   |
 * +---+---+   +   +   +---+   +---+---+   +
 * |           |   |   |       |           |
 * +   +---+---+   +   +   +   +   +   +   +
 * |   |           |   |   |   |   |   |   |
 * +---+---+---+---+---+---+---+---+---+---+
 * 
 */

#include <stdio.h>
#include <stdlib.h>

#include "grid.h"

int
treewalker(GRID *g, CELL *c, void*unused)
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
}

int
main(int notused, char**ignored)
{
  GRID *g;
  char *board;

  g = creategrid(10,10,1);
  iterategrid(g, treewalker, NULL);

  board = ascii_grid(g, 0);
  puts(board);

  free(board);
  freegrid(g);
  return 0;
}
