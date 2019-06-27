/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */

/* Fourth maze from Mazes for Programmers,
 * this is a random walk maze that produces good results
 * but is slow to generate.
 *
 * Example maze:
 *
 *
 * Named for David Bruce Wilson, this is another random walk
 * maze, but perhaps a little faster than Aldous-Broder.
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
  MASKSETTING ms;
  char *board;
  int rc;

  defaultmasksetting(&ms);
  g = creategrid(12, 12, UNVISITED);
  ms.to_visit = g->max;

  /* Mask out the corners */
  for(int i = 0; i < 5; i++) {
    for(int j = 0; j < i+1; j ++) {
      /* top left */
      c = visitrc(g, 4 - i, j);
      if(!c) { return 1; }
      c->ctype = MASKED;
      namebycell(c, ":::");
      ms.to_visit --;

      /* bottom right */
      c = visitrc(g, 7 + i, 11 - j);
      if(!c) { return 1; }
      c->ctype = MASKED;
      namebycell(c, ":::");
      ms.to_visit --;
    }
  }

  rc = wilson(g, &ms);
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
