/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */

/* Fifth maze from Mazes for Programmers,
 * this is a random walk similar to Aldous-Broder, but movement
 * is constrained to unvisited cells. This version builds the maze
 * on a partially masked off-limits grid.
 *
 * Example maze:
 *
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |:::|:::|:::|:::| x   x   x   x     |       |:::|:::|:::|:::|
 * +---+---+---+---+   +---+---+   +   +   +   +---+---+---+---+
 * |:::|:::|:::|:::| x   x   x | x |       |   |:::|:::|:::|:::|
 * +---+---+---+---+---+---+   +   +   +---+---+---+---+---+---+
 * |:::|:::|:::|:::|:::|:::| x | x |   |:::|:::|:::|:::|:::|:::|
 * +---+---+---+---+---+---+   +   +   +---+---+---+---+---+---+
 * |:::|:::|:::|:::|:::|:::| x | x |   |:::|:::|:::|:::|:::|:::|
 * +---+---+---+---+---+---+   +   +   +---+---+---+---+---+---+
 * |:::|:::|:::|:::|:::|:::| x | x |   |:::|:::|:::|:::|:::|:::|
 * +---+---+---+---+---+---+   +   +   +---+---+---+---+---+---+
 * |:::|:::|:::|:::|:::|:::| x | x |   |:::|:::|:::|:::|:::|:::|
 * +---+---+---+---+---+---+   +   +---+---+---+---+---+---+---+
 * |:::|:::|:::|:::|:::|     x | x   x   x |:::|:::|:::|:::|:::|
 * +---+---+---+---+---+---+   +---+   +   +---+---+---+---+---+
 * |:::|:::|:::|:::|   | x   x |   |   | x   x |:::|:::|:::|:::|
 * +---+---+---+---+   +   +---+   +   +---+   +---+---+---+---+
 * |:::|       | x   x | x |           |     x   x   x     |:::|
 * +---+   +---+   +   +   +   +---+---+---+---+---+   +---+---+
 * |       | x   x | x   x |   |                   | x   x     |
 * +   +---+   +   +---+---+   +   +---+   +---+---+---+   +   +
 * | x   x   x |           |       |       |     x   x   x |   |
 * +   +---+   +---+---+---+---+---+   +---+   +   +---+---+   +
 * | x   x |       |           |       |       | x | x   x |   |
 * +---+   +---+   +   +---+   +---+   +---+---+   +   +   +   +
 * | x   x |   |   |       |       |           | x   x | x |   |
 * +   +---+   +   +---+---+---+   +---+---+   +---+---+   +   +
 * | x   x   x |       |           |           | x   x   x |   |
 * +---+---+   +---+   +   +---+---+   +---+---+   +---+---+   +
 * | x   x   x |       |   |           |       | x     |       |
 * +   +---+---+---+---+   +   +---+---+---+   +   +---+   +---+
 * | x                 |       |               | x |       |   |
 * +   +---+---+---+   +---+---+   +---+---+---+   +   +   +   +
 * | x   x   x   x |               |     x   x   x |   |   |   |
 * +---+---+---+   +   +---+---+---+---+   +---+---+   +   +   +
 * | x   x |   | x |       | x   x | x   x |   |       |   |   |
 * +   +   +   +   +---+   +   +   +   +---+   +   +---+   +   +
 * | x | x   x | x |       | x | x   x | A |       |       |   |
 * +   +---+   +   +   +---+   +---+---+   +   +---+---+---+   +
 * | x   B | x   x |       | x   x   x   x |                   |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
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
  MASKSETTING ms;
  char *board;
  int rc;

  defaultmasksetting(&ms);
  g = creategrid(20, 15, UNVISITED);

  ms.to_visit = g->max;

  /* Mask out a bottle shape */
  for(int i = 0; i < 9; i++) {
    int j, js1, js2, je1, je2;
    js1 = 0;
    je2 = g->cols;

    switch (i) {
       case 0: case 1:  je1 = 4; js2 = 11; break;
       case 2: case 3:
       case 4: case 5:  je1 = 6; js2 =  9; break;
       case 6:          je1 = 5; js2 = 10; break;
       case 7:          je1 = 4; js2 = 11; break;
       case 8:          je1 = 1; js2 = 14; break;
    }
    for(j = js1; j < je1; j ++) {
      CELL *c = visitrc(g, i, j);
      if(!c) { return 1; }
      c->ctype = MASKED;
      namebycell(c, ":::");
      ms.to_visit --;
    }
    for(j = js2; j < je2; j ++) {
      CELL *c = visitrc(g, i, j);
      if(!c) { return 1; }
      c->ctype = MASKED;
      namebycell(c, ":::");
      ms.to_visit --;
    }
  } /* masking */

  rc = huntandkill(g, &ms);
  if(rc) {
    printf("Um, issue.\n");
  }

  printf("Unsolved\n");
  board = ascii_grid(g, 0);
  puts(board);
  free(board);

  CELL *c = visitid(g, 8);
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
