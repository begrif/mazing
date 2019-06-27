/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */

/* Third maze from Mazes for Programmers,
 * this is a random walk maze that produces good results
 * but is slow to generate. This one has part of the grid
 * masked off-limits.
 *
 * Example maze (longest path solved):
 *
 * +---+---+---+---+---+---+---+---+---+---+
 * |:::|:::|:::|:::|   |   |:::|:::|:::|:::|
 * +---+---+---+---+   +   +---+---+---+---+
 * |:::|:::|:::| x   x   x | B |:::|:::|:::|
 * +---+---+---+   +---+   +   +---+---+---+
 * |:::|:::|     x     | x   x     |:::|:::|
 * +---+---+---+   +   +---+---+---+---+---+
 * |:::| x   x   x |   |       |   | A |:::|
 * +---+   +---+---+---+---+   +   +   +---+
 * |   | x |               |       | x |   |
 * +   +   +---+---+   +---+   +---+   +   +
 * |     x     | x   x   x   x | x   x     |
 * +---+   +---+   +---+---+   +   +---+---+
 * |:::| x   x   x |         x   x     |:::|
 * +---+---+   +---+---+   +   +---+---+---+
 * |:::|:::|           |   |       |:::|:::|
 * +---+---+---+   +---+   +   +---+---+---+
 * |:::|:::|:::|       |   |   |:::|:::|:::|
 * +---+---+---+---+   +---+---+---+---+---+
 * |:::|:::|:::|:::|       |:::|:::|:::|:::|
 * +---+---+---+---+---+---+---+---+---+---+
 *
 *
 * Named for David Aldous and Andrei Broder, this is a random
 * walk that runs until ever cell has been visited once. It's
 * inefficient in that there are a lot of useless second, third,
 * etc, cell visits in the process.
 */

#include <stdio.h>
#include <stdlib.h>

#include "mazes.h"


int
main(int notused, char**ignored)
{
  GRID *g;
  CELL *c;
  DMAP *dm;
  char *board;
  MASKSETTING ms;
  int rc, cellcount;

  defaultmasksetting(&ms);
  g = creategrid(10,10, UNVISITED);
  cellcount = g->max;

  /* Mask out the corners */
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < i+1; j ++) {
      /* top left */
      c = visitrc(g, 3 - i, j);
      if(!c) { return 1; }
      c->ctype = MASKED;
      namebycell(c, ":::");
      cellcount --;

      /* top right */
      c = visitrc(g, 3 - i, 9 - j);
      if(!c) { return 1; }
      c->ctype = MASKED;
      namebycell(c, ":::");
      cellcount --;

      /* bottom left */
      c = visitrc(g, 6 + i, j);
      if(!c) { return 1; }
      c->ctype = MASKED;
      namebycell(c, ":::");
      cellcount --;

      /* bottom right */
      c = visitrc(g, 6 + i, 9 - j);
      if(!c) { return 1; }
      c->ctype = MASKED;
      namebycell(c, ":::");
      cellcount --;
    }
  }

  ms.to_visit = cellcount;

  rc = aldbro(g, &ms);
  if(rc) {
    printf("Um, issue.\n");
  }

  printf("Unsolved\n");
  board = ascii_grid(g, 0);
  puts(board);
  free(board);

  dm = findlongestpath(g, VISITED);
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
