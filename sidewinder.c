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
#include "mazes.h"

int
main(int notused, char**ignored)
{
  GRID *g;
  char *board;
  sw_tree_status stats;

  stats.runlength = 0;

  g = creategrid(10,10,1);
  iterategrid(g, sidewinderwalker, &stats);

  board = ascii_grid(g, 0);
  puts(board);

  free(board);
  freegrid(g);
  return 0;
}
