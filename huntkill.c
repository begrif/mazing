/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */

/* Fifth maze from Mazes for Programmers,
 * this is a random walk similar to Aldous-Broder, but movement
 * is constrained to unvisited cells.
 *
 * Example maze:
 *
 * +---+---+---+---+---+---+---+---+---+---+
 * |                                       |
 * +   +---+   +---+---+---+   +---+---+   +
 * |       |   |               |       |   |
 * +---+   +   +   +   +---+---+   +   +   +
 * |       |   |   |   |       |   |       |
 * +---+   +   +---+   +   +---+   +   +---+
 * |       |   |       |           |       |
 * +   +---+   +   +---+---+---+---+   +   +
 * |       |   |   |           |       |   |
 * +---+   +   +   +   +---+   +---+---+   +
 * |       |   |           |   |       |   |
 * +   +---+   +---+---+   +   +   +---+---+
 * |   |               |   |   |           |
 * +---+   +---+---+---+   +   +---+---+   +
 * |       |       |       |   |       |   |
 * +   +---+   +---+   +---+   +   +---+   +
 * |           |       |   |   |           |
 * +---+---+   +   +---+   +   +   +   +---+
 * |           |           |       |       |
 * +---+---+---+---+---+---+---+---+---+---+
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
  int rc;

  g = creategrid(10, 10, UNVISITED);
  rc = huntandkill(g);
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
