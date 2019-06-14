/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* testing a maze grid */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grid.h"

int
visit_should_work(GRID *g,int i, int j)
{
  CELL *c = visitrc(g, i, j);
  if(!c) {
    printf("visit() of %d,%d failed.\n", i, j);
    return(-1);
  }
  printf("cell %d,%d is id %d has type: %d\n", i, j, c->id, c->ctype);
  if(c->name) { printf("\tnamed %s\n", c->name); } 
         else { printf("\tnot named\n"); }

  for(int d = 0; d < FOURDIRECTIONS; d++) {
    int id = c->dir[d];
    if(id != NC) {
       printf("\tdir %s connects to %d\n", dirtoname(d), c->dir[d]);
    } else {
       printf("\tnot connected to %s\n", dirtoname(d));
    }
  }
  return(0);
}

int
visit_should_fail(GRID* g,int i, int j)
{
  CELL *c = visitrc(g, i, j);
  if(c) {
    printf("visit() of %d,%d failed.\n", i, j);
    return(-1);
  }
  return(0);
}

int
tryconnect(GRID *g, CELL *c, int d)
{
    CELL *nc = visitdir(g, c, d, ANY);

    if(nc) { 
      printf("Connecting %d to %d (%s)\n", c->id, nc->id, dirtoname(d));
      connectbycell(c, d, nc, opposite(d));

      if((c->dir[d] != nc->id) || (nc->dir[opposite(d)] != c->id)) {
	 printf("that connection failed\n");
	 printf("c->dir[d] != nc->id: c->dir[%s] != %d\n",
	 		dirtoname(d), nc->id);
	 printf("nc->dir[d] != c->id: nc->dir[%s] != %d\n",
	 		dirtoname(opposite(d)), c->id);
	 return(6);
      }
    } else {
      printf("Poor choice of starting cell.\n");
      return(1);
    }
    return(0);
}

int
printidandname(GRID *notused, CELL *c, void *unused)
{
  if(!c) { 
    printf("Not a cell\n");
    return -100;
  }
  printf("Cell id %d at (%d,%d)", c->id, c->row, c->col);
  if(c->name) {
    printf(" is named %s\n", c->name);
  } else {
    printf(" has no name\n");
  }
  return 1;
}

typedef struct { int total; } insum;

int
counter(GRID *notused, CELL *c, void *t_p)
{
  insum *t;
  t = (insum *)t_p;
  t->total ++;

  if(!c) { 
    return -100;
  }
   
  return 1;
}

int
main(int ignored, char**notused)
{
  GRID *g;
  CELL *c1, *c2, *c3, *c4;
  int d, edges, walls;
  int gr, gc;
  int mr1, mr2, mc1, mc2;
  int id, rc;
  insum total = { 0 };
  char *board;
  const char expectedboard[] = "+---+---+---+\n"
                               "|   |   |   |\n"
                               "+---+   +---+\n" 
                               "|     X     |\n"
                               "+---+   +---+\n" 
                               "|   |   |   |\n"
                               "+---+---+---+\n";

  gr = 4; gc = 6;

  g = creategrid(gr,gc,1);
  if(!g) {
    printf("creategrid( %d x %d ) failed.\n", gr, gc);
    return(1);
  }

  if(visit_should_fail(g, -1, 2)) {
    printf("out of bounds test failed\n");
    return(2);
  }
  if(visit_should_fail(g, 18, 1)) {
    printf("out of bounds test failed\n");
    return(2);
  }
  if(visit_should_fail(g, 1, 18)) {
    printf("out of bounds test failed\n");
    return(2);
  }

  if(visit_should_work(g, 0, 0)) {
    printf("real cell visit failed\n");
    return(3);
  }
  if(visit_should_work(g, gr-1, gc-1)) {
    printf("real cell visit failed\n");
    return(3);
  }

  printf("Bulk naming with W/E connect on odd rows\n");
  for(int col = 0; col < gc; col ++) {
    for(int row = 0; row < gr; row ++) {
      char buf[80];
      sprintf(buf, "%cx%c", 'a'+row, 'a'+col);
      namebyrc(g, row, col, buf);
      if(col && (row % 2)) {
        connectbyrc(g, row, col-1, EAST, row, col, WEST);
      }
    }
  }

  mc1 = mc2 = 1;
  mr1 = 2;
  mr2 = mr1 + 1;
  c1 = visitrc(g,mr1,mc1);
  if(!c1) { 
    printf("first cell lookup for manual connection failed\n");
    return(4);
  }
  namebycell(c1, "C1: starting point");
  id = c1->id;
  c4 = visitid(g,id);
  if((c4->row != mr1) || (c4->col != mc1)) {
    printf("visitbyid didn't match\n");
    return(4);
  }

  c2 = visitrc(g,mr2,mc2);
  namebycell(c2, "C2: ending point");
  if(!c2) { 
    printf("second cell lookup for manual connection failed\n");
    return(4);
  }

  printf("Adding a connection to SOUTH of C1\n");
  connectbycell(c1, SOUTH, c2, SYMMETRICAL);

  printf("Checking connection from southern cell\n");
  if(visit_should_work(g, mr2, mc2)) {
    printf("real cell visit failed\n");
    return(4);
  }
  d = isconnectedbycell(c2, c1, NORTH);
  if( d != NORTH ) {
    printf("cells not connected as expected\n");
  }
  
  c3 = visitdir(g, c1, SOUTH, THIS);
  if((!c3) || (c3->row != mr2) || (c3->col != mc2)) {
    printf("directional cell visit failed\n");
    if(!c3) {
      printf("no c3\n");
    } else {
      printf("c3->row %d != %d || c3->col %d != %d\n",c3->row,mr2,c3->col,mc2);
    }
    return(4);
  }

  printf("Now trying to directly visit NORTH from id %d.\n", c2->id);
  c1 = visitdir(g, c2, NORTH, SYMMETRICAL);
  if(!c1) {
    printf("That visit failed.\n");
    return(4);
  } else {
    d = isconnectedbycell(c1, c2, ANY);
    printf("That visit the direction from c1 to c2 is %s.\n", dirtoname(d));
    if (d != SOUTH) {
      printf("Whelp, that failed\n");
      return(4);
    }
  }

  c1 = visitrandom(g);
  if(!c1) {
    printf("Random visit failed.\n");
    return(5);
  } else {
    int rc;

    printf("Found %d randomly\n", c1->id);
    namebyid(g, c1->id, "RND");
    printf("Printing details of row %d\n", c1->row);
    rc = iteraterow(g, c1->row, printidandname, NULL);
    if(rc != g->cols) {
      printf("Not enough cols. rc %d != %d\n", rc, g->cols);
      return(5);
    }

    printf("Printing details of col %d\n", c1->col);
    rc = iteratecol(g, c1->col, printidandname, NULL);
    if(rc != g->rows) {
      printf("Not enough rows. rc %d != %d\n", rc, g->rows);
      return(5);
    }
  }

  printf("Now finding north of old C1 point.\n");
  c2 = visitdir(g, c4, NORTH, ANY);
  if(!c2) {
    printf("Didn't work. ugh.\n");
    return(6);
  }


  for(int d = FIRSTDIR; d < FOURDIRECTIONS; d++) {
    if(tryconnect(g, c2, d)) {
      return(6);
    }
  }
  
  rc = iterategrid(g, counter, &total);
  if(rc != total.total) {
    printf("Iterate grid got inconsistent results, %d != %d\n", rc, total.total);
    return(7);
  } else {
    printf("Iterated over whole grid\n");
  }

  printf("\nVisual only test of ascii_grid\n");
  namegrid(g, "Test board");
  board = ascii_grid(g, 1);
  puts(board);
  free(board);

  freegrid(g);

  printf("\nNew 3x3 grid\n");

  g = creategrid(3,3,2);
  c1 = visitrc(g,1,1);
  edges = edgestatusbycell(g,c1);
  walls = wallstatusbycell(c1);

  if(edges == NO_EDGES) {
    printf("No edges on middle cell, correct.\n");
  } else {
    printf("Found edges on middle cell, wrong.\n");
    return(7);
  }

  if(walls == NO_WALLS) {
    printf("Missing walls on middle cell, wrong.\n");
    return(7);
  } else {
    printf("No walls on middle cell, correct.\n");
    if(walls & NORTH_WALL) { } else { printf("But missing north\n"); return 7;}
    if(walls & SOUTH_WALL) { } else { printf("But missing south\n"); return 7;}
    if(walls & EAST_WALL ) { } else { printf("But missing east\n"); return 7;}
    if(walls & WEST_WALL ) { } else { printf("But missing west\n"); return 7;}
  }

  edges = edgestatusbyid(g,0);
  if(edges == NORTHWEST_CORNER) {
    printf("NW corner cell, correct.\n");
  } else {
    printf("Wrong edges on NW corner\n");
    return(7);
  }

  edges = edgestatusbyrc(g,2,2);
  if(edges == SOUTHEAST_CORNER) {
    printf("SE corner cell, correct.\n");
  } else {
    printf("Wrong edges on SE corner\n");
    return(7);
  }

  printf("Knocking down all walls on middle cell.\n");
  for(int d = FIRSTDIR; d < FOURDIRECTIONS; d++) {
    if(tryconnect(g, c1, d)) {
      return(6);
    }
  }
  namebycell(c1, " X");
  walls = wallstatusbycell(c1);
  if(walls == NO_WALLS) {
    printf("Now correctly no walls\n");
  } else {
    printf("What? Still walls on middle cell, wrong.\n");
    return(7);
  }

  board = ascii_grid(g, 1);
  puts(board);
  if(0 == strncmp(board, expectedboard, BUFSIZ)) {
    printf("ASCII art as expected\n");
  } else {
    printf("ASCII art wrong\n");
    return(7);
  }

  freegrid(g);
  return(0);
}
