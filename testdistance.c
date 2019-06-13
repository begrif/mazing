/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */

#include <stdio.h>
#include <stdlib.h>

#include "grid.h"
#include "distance.h"

/* Not a maze, but a serpentine path, for testing the
 * worst case of a path from point A to B.
 *
 * Example maze:
 *
 * +---+---+---+---+---+---+---+---+---+---+
 * |                                     A |
 * +   +---+---+---+---+---+---+---+---+---+
 * |                                       |
 * +---+---+---+---+---+---+---+---+---+   +
 * |                                       |
 * +   +---+---+---+---+---+---+---+---+---+
 * |                                     B |
 * +---+---+---+---+---+---+---+---+---+---+
 *
 */
int
serpentine(GRID *g, CELL *c, void*unused)
{
  CELL *cc;
  int edges;
  int go;

  if(!g) { return -1; }
  if(!c) { return -1; }

  edges = edgestatusbycell(g,c);

  if(edges & EAST_EDGE) {
    if(c->row % 2) {
      cc = visitdir(g, c, SOUTH, ANY);
      connectbycell(c, SOUTH, cc, SYMMETRICAL);
    }
  }

  if(edges & WEST_EDGE) {
    if(0 == (c->row % 2)) {
      cc = visitdir(g, c, SOUTH, ANY);
      connectbycell(c, SOUTH, cc, SYMMETRICAL);
    }
  }

  if(!(edges & EAST_EDGE)) {
    cc = visitdir(g, c, EAST, ANY);
    connectbycell(c, EAST, cc, SYMMETRICAL);
  }
  return 0;
}

/* Other direction worst case, no walls at all.
 * Example maze:
 * +---+---+---+---+---+---+---+
 * |                           |
 * +   +   +   +   +   +   +   +
 * |                           |
 * +   +   +   +   +   +   +   +
 * |                           |
 * +   +   +   +   +   +   +   +
 * |                           |
 * +---+---+---+---+---+---+---+
 */
int
hollow(GRID *g, CELL *c, void*unused)
{
  CELL *cc;
  int edges;
  int go;

  if(!g) { return -1; }
  if(!c) { return -1; }

  edges = edgestatusbycell(g,c);

  if(!(edges & EAST_EDGE)) {
    cc = visitdir(g, c, EAST, ANY);
    connectbycell(c, EAST, cc, SYMMETRICAL);
  }

  if(!(edges & SOUTH_EDGE)) {
    cc = visitdir(g, c, SOUTH, ANY);
    connectbycell(c, SOUTH, cc, SYMMETRICAL);
  }

  return 0;
}

int
printpath(TRAIL *walk, int max)
{
  int steps = 0;
  if(!walk) { return 1; }
  
  do {
    printf("step %d on id(%d); ", steps++, walk->cell_id);
    if(steps == max) {
      printf("\nRunaway\n");
      return 2;
    }
  } while( (walk = walk->next) );

  printf(" done\n");
  return 0;
}

int
main(int notused, char**ignored)
{
  GRID *g;
  DMAP *dm;
  char *board;
  int distance;
  int rc;

  g = creategrid(10,10,1);
  if(!g) {
    printf("Create serpentine grid failed.\n");
    return 1;
  }
  iterategrid(g, serpentine, NULL);

  namebyid(g,  9, " A");
  namebyid(g, 99, " B");

  board = ascii_grid(g, 1);
  puts(board);
  free(board);

  dm = createdistancemap(g, visitid(g,9) );
  if(!dm) {
    printf("Create distancemap failed.\n");
    return 1;
  }

  distance = distanceto(dm, visitid(g,99) );
  ascii_dmap(dm);

  if(distance != 99) {
    printf("Find distance failed %d\n", distance);
    return 1;
  }
  printf("Distance is correctly %d\n", distance);

  rc = findpath(dm);
  if( rc != 0 ) {
    printf("Find path failed %d\n", rc);
    return 1;
  }
  rc = printpath(dm->path, 101);
  if( rc != 0 ) {
    printf("printpath failed %d\n", rc);
    return 1;
  }

  freedistancemap(dm);
  freegrid(g);

  g = creategrid(5,10,1);
  if(!g) {
    printf("Create hollow grid failed.\n");
    return 2;
  }
  iterategrid(g, hollow, NULL);
  namebyid(g,  0, " A");
  namebyid(g, g->max - 1, " B");

  board = ascii_grid(g, 1);
  puts(board);
  free(board);

  dm = createdistancemap(g, visitid(g,0) );
  if(!dm) {
    printf("Create distancemap failed.\n");
    return 2;
  }

  distance = distanceto(dm, visitid(g,g->max - 1) );
  ascii_dmap(dm);

  /* 12 because (width+height-2) when no diagonal movement  */
  if(distance != g->cols + g->rows -2) {
    printf("Find distance failed %d\n", distance);
    return 2;
  }
  printf("Distance is correctly %d\n", distance);

  rc = findpath(dm);
  if( rc != 0 ) {
    printf("Find path failed %d\n", rc);
    return 2;
  }
  rc = printpath(dm->path, distance+2);
  if( rc != 0 ) {
    printf("printpath failed %d\n", rc);
    return 2;
  }

  freedistancemap(dm);
  freegrid(g);

  g = creategrid(3,3,1);
  if(!g) {
    printf("Create pathless grid failed.\n");
    return 3;
  }
  namebyid(g,  0, " A");
  namebyid(g, g->max - 1, " B");

  board = ascii_grid(g, 1);
  puts(board);
  free(board);

  dm = createdistancemap(g, visitid(g,0) );
  if(!dm) {
    printf("Create distancemap failed.\n");
    return 3;
  }

  distance = distanceto(dm, visitid(g,g->max - 1) );
  ascii_dmap(dm);

  if(distance != DISTANCE_ERROR) {
    printf("Find distance failed %d\n", distance);
    return 3;
  }
  printf("Distance correctly failed with %d\n", distance);

  rc = findpath(dm);
  if( rc == 0 ) {
    printf("findpath should have failed %d\n", rc);
    return 3;
  }
  printf("findpath correctly failed with %d\n", rc);

  freedistancemap(dm);
  freegrid(g);

  return 0;
}
