/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */

/* Fourth maze from Mazes for Programmers,
 * this is a random walk maze that produces good results
 * but is slow to generate.
 *
 * Example maze:
 *
 * +---+---+---+---+---+---+---+---+---+---+
 * |   |       |           |           |   |
 * +   +   +   +   +   +---+---+   +   +   +
 * |       |       |               |   |   |
 * +---+---+---+---+---+---+---+   +---+   +
 * |                   |           |       |
 * +   +---+   +---+---+---+   +   +---+   +
 * |   |       |           |   |   |   |   |
 * +---+---+   +   +---+   +---+   +   +   +
 * |               |                       |
 * +---+---+   +   +   +---+   +---+---+   +
 * |           |   |       |   |           |
 * +   +---+   +---+   +---+---+---+   +   +
 * |   |       |       |               |   |
 * +   +---+---+---+   +---+   +   +---+---+
 * |   |           |       |   |           |
 * +   +---+---+   +   +---+   +---+   +---+
 * |   |   |       |       |   |   |       |
 * +   +   +   +   +   +---+---+   +---+---+
 * |   |       |                           |
 * +---+---+---+---+---+---+---+---+---+---+
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
  DMAP *dm;
  char *board;
  int rc;

  g = creategrid(10, 10, UNVISITED);
  rc = wilson(g, NULL);
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
