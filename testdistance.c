/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mazes.h"

/* This uses two non-random "maze" structures for test.
 *
 * First a serpentine path, for testing the
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
 * Second, other direction worst case, hollow with no walls at all.
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
  CELL *c;
  DMAP *dm;
  char *board;
  int distance;
  int rc;
  int errorgroup = 1;

  g = creategrid(10,10,1);
  if(!g) {
    printf("Create serpentine grid failed.\n");
    return errorgroup;
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
    return errorgroup;
  }

  distance = distanceto(dm, visitid(g,99), 1);
  ascii_dmap(dm);

  if(distance != 99) {
    printf("Find distance failed %d\n", distance);
    return errorgroup;
  }
  printf("Distance is correctly %d\n", distance);

  rc = findpath(dm);
  if( rc != 0 ) {
    printf("Find path failed %d\n", rc);
    return errorgroup;
  }
  rc = printpath(dm->path, 101);
  if( rc != 0 ) {
    printf("printpath failed %d\n", rc);
    return errorgroup;
  }

  freedistancemap(dm);
  freegrid(g);
  errorgroup ++;

  g = creategrid(5,10,1);
  if(!g) {
    printf("Create hollow grid failed.\n");
    return errorgroup;
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
    return errorgroup;
  }

  distance = distanceto(dm, visitid(g,g->max - 1), 1);
  ascii_dmap(dm);

  /* 12 because (width+height-2) when no diagonal movement  */
  if(distance != g->cols + g->rows -2) {
    printf("Find distance failed %d\n", distance);
    return errorgroup;
  }
  printf("Distance is correctly %d\n", distance);

  rc = findpath(dm);
  if( rc != 0 ) {
    printf("Find path failed %d\n", rc);
    return errorgroup;
  }
  rc = printpath(dm->path, distance+2);
  if( rc != 0 ) {
    printf("printpath failed %d\n", rc);
    return errorgroup;
  }

  freedistancemap(dm);
  freegrid(g);
  errorgroup ++;

  g = creategrid(3,3,1);
  if(!g) {
    printf("Create pathless grid failed.\n");
    return errorgroup;
  }
  namebyid(g,  0, " A");
  namebyid(g, g->max - 1, " B");

  board = ascii_grid(g, 1);
  puts(board);
  free(board);

  dm = createdistancemap(g, visitid(g,0) );
  if(!dm) {
    printf("Create distancemap failed.\n");
    return errorgroup;
  }

  distance = distanceto(dm, visitid(g,g->max - 1), 1);
  ascii_dmap(dm);

  if(distance != DISTANCE_ERROR) {
    printf("Find distance failed %d\n", distance);
    return errorgroup;
  }
  printf("Distance correctly failed with %d\n", distance);

  rc = findpath(dm);
  if( rc == 0 ) {
    printf("findpath should have failed %d\n", rc);
    return errorgroup;
  }
  printf("findpath correctly failed with %d\n", rc);

  freedistancemap(dm);

  iterategrid(g, serpentine, NULL);
  board = ascii_grid(g, 1);
  puts(board);
  free(board);

  dm = createdistancemap(g, visitid(g,0) );
  distance = distanceto(dm, visitid(g,g->max - 1), 0);
  ascii_dmap(dm);

  if(dm->farthest != 6) {
    printf("find farthest got wrong answer: %d\n", dm->farthest);
    return errorgroup;
  }
  printf("find farthest got to %d, distance %d\n",
  	dm->farthest_id, dm->farthest);
  freedistancemap(dm);

  dm = findlongestpath(g, g->gtype);
  if(!dm) {
    printf("findlongestpath failed\n");
    return errorgroup;
  }
  printf("findlongestpath found:\n");
  ascii_dmap(dm);
  rc = printpath(dm->path, 10);
  if( rc != 0 ) {
    printf("printpath failed %d\n", rc);
    return errorgroup;
  }
  if((dm->farthest != 8) || (dm->farthest_id != 2)) {
    printf("incorrect longest path\n");
    return errorgroup;
  }
  printf("a longest possible path found\n");
  
  namepath(dm, "STA", NULL, "END");
  board = ascii_grid(g, 1);
  puts(board);
  free(board);

  c = visitid(g,1);
  if(!c) {
    printf("something broke with the grid\n");
    return errorgroup;
  }
  if(strncmp("7", c->name, 1)) {
    printf("unexpected name on cell %d: %s\n", c->id, c->name);
    return errorgroup;
  }
  c = visitid(g,2);
  if(!c) {
    printf("something broke with the grid\n");
    return errorgroup;
  }
  if(strncmp("END", c->name, 4)) {
    printf("unexpected name on cell %d: %s\n", c->id, c->name);
    return errorgroup;
  }
  printf("naming iterator works\n");

  freedistancemap(dm);
  freegrid(g);
  errorgroup ++;

  printf("\nCreating micro grid to test path.\n");
  g = creategrid(1,1,3);
  if(!g) {
    printf("Create micro grid failed.\n");
    return errorgroup;
  }
  dm = findlongestpath(g, g->gtype);
  if(!dm) {
    printf("findlongestpath microgrid failed\n");
    return errorgroup;
  }
  rc = printpath(dm->path, 2);
  if( rc != 0 ) {
    printf("printpath failed %d\n", rc);
    return errorgroup;
  }
  if(dm->path->cell_id != 0) {
    printf("findlongestpath microgrid didn't work\n");
  }
  printf("findlongestpath microgrid worked\n");

  freedistancemap(dm);
  freegrid(g);
  errorgroup ++;

  printf("\nCreating super simple grid to test masking.\n");
  g = creategrid(1,5, MASKED);
  if(!g) {
    printf("Create masking grid failed.\n");
    return errorgroup;
  }
  c = visitid(g, 2); 
  if(!c) {
    printf("Cell error\n");
    return errorgroup;
  }
  c->ctype = VISITED;
  dm = findlongestpath(g, VISITED);
  if(!dm) {
    printf("findlongestpath masked grid failed\n");
    return errorgroup;
  }
  rc = printpath(dm->path, 2);
  if( rc != 0 ) {
    printf("printpath failed %d\n", rc);
    return errorgroup;
  }
  if(dm->path->cell_id != c->id) {
    printf("findlongestpath masked grid didn't work\n");
  }
  printf("findlongestpath masked grid worked\n");
  freedistancemap(dm);
  freegrid(g);

  errorgroup ++;

  g = creategrid(7,3, VISITED);
  if(!g) { 
    printf("7x3 grid failed %d\n", rc);
    return errorgroup;
  }
  printf("7x3 grid for rotated path test\n");
  iterategrid(g, serpentine, NULL);
  board = ascii_grid(g, 1);
  puts(board);
  free(board);

  for (int rot = 0; rot < 360; rot += 90 ) {
    dm = findlongestpath(g, VISITED);
    if(!dm) {
      printf("findlongestpath rotated grid failed %d degrees\n", rot);
      board = ascii_grid(g, 1);
      puts(board);
      free(board);
      return errorgroup;
    }
    printf("Farthest on rotation %d is %d\n", rot, dm->farthest);
    if( rotategrid(g, CLOCKWISE) ) {
      printf("rotate 7x3 failed (round %d)\n", rot);
    }
  }
  freedistancemap(dm);
  freegrid(g);

  return 0;
}
