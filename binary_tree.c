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
#include "mazes.h"

int
main(int notused, char**ignored)
{
  GRID *g;
  char *board;

  g = creategrid(10,10,1);
  iterategrid(g, btreewalker, NULL);

  board = ascii_grid(g, 0);
  puts(board);

  free(board);
  freegrid(g);
  return 0;
}
