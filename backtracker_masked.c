/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */

/* This is the backtracker, but with some cells masked off.
 *
 * Example maze:
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+
 * | x   x     | x   x         | x   x   x   x   x     |
 * +   +   +---+   +   +---+   +   +---+---+---+   +   +
 * | B | x   x   x | x   x |   | x |           | x |   |
 * +---+---+---+---+   +   +---+   +   +---+---+   +   +
 * | x   x   x   A |   | x   x   x     | x   x | x |   |
 * +   +---+---+---+   +---+---+---+   +   +   +   +   +
 * | x   x   x   x |   |:::|:::|:::|   | x | x | x |   |
 * +---+---+---+   +---+---+---+---+---+   +   +   +   +
 * |           | x |:::|:::|:::|:::|:::| x | x   x |   |
 * +   +   +---+   +---+---+---+---+---+   +---+---+---+
 * |   |   | x   x |:::|:::|:::|:::|:::| x   x   x   x |
 * +   +---+   +---+---+---+---+---+---+---+   +---+   +
 * |   | x   x |:::|:::|:::|:::|:::|:::|:::|   | x   x |
 * +   +   +---+---+---+---+---+---+---+---+---+   +   +
 * |   | x |       |:::|:::|:::|:::|:::| x   x   x |   |
 * +   +   +---+   +---+---+---+---+---+   +---+---+   +
 * |   | x   x   x |:::|:::|:::|:::|:::| x   x |       |
 * +   +---+---+   +---+---+---+---+---+   +   +---+   +
 * |           | x   x |:::|:::|:::|       | x   x |   |
 * +   +---+   +---+   +---+---+---+---+---+   +   +---+
 * |   |       | x   x | x   x   x   x   x |   | x   x |
 * +   +   +---+   +---+   +---+---+---+   +---+   +   +
 * |   |       | x   x   x |           | x   x |   | x |
 * +   +---+   +---+---+---+   +---+   +---+   +---+   +
 * |       |                       |         x   x   x |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "mazes.h"


int
main(int notused, char**ignored)
{
  GRID *g;
  DMAP *dm;
  char *board;
  int rc, cellcount;

  g = creategrid(13, 13, UNVISITED);
  if(!g) { return 1; }
  cellcount = g->max;

  /* Mask out a circle (roughly) in the center */
  for(int i = 3; i < 10; i++) {
    int js, je;
    switch (i) {
      case 3: case 9: js = 5; je =  8; break;
      case 4: case 8: js = 4; je =  9; break;
      case 5: case 7: js = 4; je =  9; break;
              case 6: js = 3; je = 10; break;
    }
    for(int j = js; j < je; j ++) {
      CELL *c = visitrc(g, i, j);
      if(!c) { return 1; }
      c->ctype = MASKED;
      namebycell(c, ":::");
      cellcount --;
    }
  }

  rc = backtracker(g, cellcount);
  if(rc) {
    printf("Um, issue.\n");
  }

  dm = findlongestpath(g, VISITED);
  if(!dm) {
    printf("Ooops, solver broke\n");
  }
  namepath(dm, " A", " x", " B");

  board = ascii_grid(g, 1);
  puts(board);
  free(board);

  freegrid(g);
  return 0;
}
