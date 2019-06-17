/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* testing a maze grid */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grid.h"

#define NONCENTER 11
#define CENTERED 17

/* for the case where board output is informational only */
void
printboard(GRID *g, int opt)
{
  char *board;
  board = ascii_grid(g, opt);
  puts(board);
  free(board);
}

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
    int check;
    CELL *nc = visitdir(g, c, d, ANY);

    if(nc) { 
      check = natdirectionbycell(c, nc);
      if( d != check ) {
        printf("Not the right natural direction result, %d != %d\n", d, check);
	return 6;
      }
      printf("Connecting %d to %d (%s direction verified)\n", c->id, nc->id, dirtoname(d));
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
counter(GRID *g, CELL *c, void *t_p)
{
  char scratch[BUFSIZ];

  insum *t;
  t = (insum *)t_p;
  t->total ++;

  if(!c) { 
    return -100;
  }

  /* the original id is used for testing rotations */
  c->ctype = c->id;
  snprintf(scratch, BUFSIZ, " %d", c->id); 
  namebyid(g, c->id, scratch); 
   
  /* the return value is used when testing iterategrid() */
  return 1;
}

GRID *
rotationtestgrid()
{
  GRID *g;
  insum total = { 0 };
  g = creategrid(2,5,1);
  if(!g) { return(GRID*)NULL; }
  iterategrid(g, counter, &total);
  return g;
}

int
checkcell(GRID *g, int id, int t)
{
  CELL *c = visitid(g, id);
  if(!c) { return NC; }

  if(c->ctype != t) {
    printf("Expected new id %d to be old id %d, but it's %d\n",
    	id, t, c->ctype);
    return NC;
  }
  return 0;
}

int
main(int ignored, char**notused)
{
  GRID *g;
  CELL *c1, *c2, *c3, *c4;
  int d, edges, walls, count;
  int gr, gc;
  int mr1, mr2, mc1, mc2;
  int id, rc, errorblock = 1;
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
    return(errorblock);
  }

  if(visit_should_fail(g, -1, 2)) {
    printf("out of bounds test failed\n");
    return(errorblock);
  }
  if(visit_should_fail(g, 18, 1)) {
    printf("out of bounds test failed\n");
    return(errorblock);
  }
  if(visit_should_fail(g, 1, 18)) {
    printf("out of bounds test failed\n");
    return(errorblock);
  }

  if(visit_should_work(g, 0, 0)) {
    printf("real cell visit failed\n");
    return(errorblock);
  }
  if(visit_should_work(g, gr-1, gc-1)) {
    printf("real cell visit failed\n");
    return(errorblock);
  }

  errorblock ++;

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
    return(errorblock);
  }
  namebycell(c1, "C1: starting point");
  id = c1->id;
  c4 = visitid(g,id);
  if((c4->row != mr1) || (c4->col != mc1)) {
    printf("visitbyid didn't match\n");
    return(errorblock);
  }

  c2 = visitrc(g,mr2,mc2);
  namebycell(c2, "C2: ending point");
  if(!c2) { 
    printf("second cell lookup for manual connection failed\n");
    return(errorblock);
  }

  printf("Adding a connection to SOUTH of C1\n");
  connectbycell(c1, SOUTH, c2, SYMMETRICAL);

  printf("Checking connection from southern cell\n");
  if(visit_should_work(g, mr2, mc2)) {
    printf("real cell visit failed\n");
    return(errorblock);
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
    return(errorblock);
  }

  printf("Now trying to directly visit NORTH from id %d.\n", c2->id);
  c1 = visitdir(g, c2, NORTH, SYMMETRICAL);
  if(!c1) {
    printf("That visit failed.\n");
    return(errorblock);
  } else {
    d = isconnectedbycell(c1, c2, ANY);
    printf("That visit the direction from c1 to c2 is %s.\n", dirtoname(d));
    if (d != SOUTH) {
      printf("Whelp, that failed\n");
      return(errorblock);
    }
  }

  errorblock ++;

  c1 = visitrandom(g);
  if(!c1) {
    printf("Random visit failed.\n");
    return(errorblock);
  } else {
    int rc;

    printf("Found %d randomly\n", c1->id);
    namebyid(g, c1->id, "RND");
    printf("Printing details of row %d\n", c1->row);
    rc = iteraterow(g, c1->row, printidandname, NULL);
    if(rc != g->cols) {
      printf("Not enough cols. rc %d != %d\n", rc, g->cols);
      return(errorblock);
    }

    printf("Printing details of col %d\n", c1->col);
    rc = iteratecol(g, c1->col, printidandname, NULL);
    if(rc != g->rows) {
      printf("Not enough rows. rc %d != %d\n", rc, g->rows);
      return(errorblock);
    }
  }

  printf("Now finding north of old C1 point.\n");
  c2 = visitdir(g, c4, NORTH, ANY);
  if(!c2) {
    printf("Didn't work. ugh.\n");
    return(errorblock);
  }

  errorblock ++;


  for(int d = FIRSTDIR; d < FOURDIRECTIONS; d++) {
    if(tryconnect(g, c2, d)) {
      return(errorblock);
    }
  }
  
  rc = iterategrid(g, counter, &total);
  if(rc != total.total) {
    printf("Iterate grid got inconsistent results, %d != %d\n", rc, total.total);
    return(errorblock);
  } else {
    printf("Iterated over whole grid\n");
  }

  printf("\nVisual only test of ascii_grid\n");
  namegrid(g, "Test board");
  printboard(g, 1);

  freegrid(g);

  printf("\nNew 3x3 grid\n");
  errorblock ++;

  g = creategrid(3,3,NONCENTER);
  c1 = visitrc(g,1,1);
  edges = edgestatusbycell(g,c1);
  walls = wallstatusbycell(c1);

  if(edges == NO_EDGES) {
    printf("No edges on middle cell, correct.\n");
  } else {
    printf("Found edges on middle cell, wrong.\n");
    return(errorblock);
  }

  if(walls == NO_WALLS) {
    printf("Missing walls on middle cell, wrong.\n");
    return(errorblock);
  } else {
    printf("No walls on middle cell, correct.\n");
    if(walls & NORTH_WALL) { } else { printf("But missing north\n"); return errorblock;}
    if(walls & SOUTH_WALL) { } else { printf("But missing south\n"); return errorblock;}
    if(walls & EAST_WALL ) { } else { printf("But missing east\n"); return errorblock;}
    if(walls & WEST_WALL ) { } else { printf("But missing west\n"); return errorblock;}
  }

  edges = edgestatusbyid(g,0);
  if(edges == NORTHWEST_CORNER) {
    printf("NW corner cell, correct.\n");
  } else {
    printf("Wrong edges on NW corner\n");
    return(errorblock);
  }
  errorblock ++;

  edges = edgestatusbyrc(g,2,2);
  if(edges == SOUTHEAST_CORNER) {
    printf("SE corner cell, correct.\n");
  } else {
    printf("Wrong edges on SE corner\n");
    return(errorblock);
  }

  printf("Knocking down all walls on middle cell.\n");
  for(int d = FIRSTDIR; d < FOURDIRECTIONS; d++) {
    if(tryconnect(g, c1, d)) {
      return(errorblock);
    }
  }
  namebycell(c1, " X");
  walls = wallstatusbycell(c1);
  if(walls == NO_WALLS) {
    printf("Now correctly no walls\n");
  } else {
    printf("What? Still walls on middle cell, wrong.\n");
    return(errorblock);
  }

  errorblock ++;
  count = ncountbycell(g, c1, EXITS, 0);
  if(count == 4) {
    printf("Exit count correct (center)\n");
  } else {
    printf("Exit count wrong (center), want 4 got %d\n",count);
    return(errorblock);
  }
  count = ncountbycell(g, c1, NEIGHBORS, 0);
  if(count == 4) {
    printf("Neighbor count correct (center)\n");
  } else {
    printf("Neighbor count wrong (center), want 4 got %d\n",count);
    return(errorblock);
  }
  c1->ctype = CENTERED;
  c1 = visitrc(g,0,1);
  count = ncountbycell(g, c1, EXITS, 0);
  if(count == 1) {
    printf("Exit count correct (top-mid)\n");
  } else {
    printf("Exit count wrong (top-mid), want 1 got %d\n",count);
    return(errorblock);
  }
  count = ncountbycell(g, c1, NEIGHBORS, 0);
  if(count == 3) {
    printf("Neighbor count correct (top-mid)\n");
  } else {
    printf("Neighbor count wrong (top-mid), want 3 got %d\n",count);
    return(errorblock);
  }
  count = ncountbycell(g, c1, OF_TYPE, CENTERED);
  if(count == 1) {
    printf("Visited neighbor count correct (top-mid)\n");
  } else {
    printf("Visited neighbor count wrong (top-mid), want 1 got %d\n",count);
    return(errorblock);
  }

  errorblock ++;


  board = ascii_grid(g, 1);
  puts(board);
  if(0 == strncmp(board, expectedboard, BUFSIZ)) {
    printf("ASCII art as expected\n");
  } else {
    printf("ASCII art wrong\n");
    return(errorblock);
  }
  free(board);

  freegrid(g);

  errorblock ++;

  g = rotationtestgrid();
  if(!g) { printf("rotation board create failed\n"); return(errorblock); }
  printf("Unrotated\n");
  printboard(g,1);

  rc = rotategrid(g, ROTATE_180);
  if(rc) { printf("rotation board failed\n"); return(errorblock); }
  printf("ROTATE_180\n");
  printboard(g,1);
  if( checkcell(g, 0, 9) || checkcell(g, 4, 5) || checkcell(g, 5, 4)) {
    printf("Incorrect.\n");  return(errorblock); 
  }
  freegrid(g);

  g = rotationtestgrid();
  if(!g) { printf("rotation board create failed\n"); return(errorblock); }
  rc = rotategrid(g, FLIP_LEFTRIGHT);
  if(rc) { printf("rotation board failed\n"); return(errorblock); }
  printf("FLIP_LEFTRIGHT\n");
  printboard(g,1);
  if( checkcell(g, 0, 4) || checkcell(g, 4, 0) || checkcell(g, 5, 9)) {
    printf("Incorrect.\n");  return(errorblock); 
  }
  freegrid(g);

  g = rotationtestgrid();
  if(!g) { printf("rotation board create failed\n"); return(errorblock); }
  rc = rotategrid(g, FLIP_TOPBOTTOM);
  if(rc) { printf("rotation board failed\n"); return(errorblock); }
  printf("FLIP_TOPBOTTOM\n");
  printboard(g,1);
  if( checkcell(g, 0, 5) || checkcell(g, 4, 9) || checkcell(g, 5, 0)) {
    printf("Incorrect.\n");  return(errorblock); 
  }
  freegrid(g);

  g = rotationtestgrid();
  if(!g) { printf("rotation board create failed\n"); return(errorblock); }
  rc = rotategrid(g, CLOCKWISE);
  if(rc) { printf("rotation board failed\n"); return(errorblock); }
  printf("CLOCKWISE\n");
  printboard(g,1);
  if( checkcell(g, 0, 5) || checkcell(g, 4, 7) || checkcell(g, 5, 2)) {
    printf("Incorrect.\n");  return(errorblock); 
  }
  freegrid(g);

  g = rotationtestgrid();
  if(!g) { printf("rotation board create failed\n"); return(errorblock); }
  rc = rotategrid(g, COUNTERCLOCKWISE);
  if(rc) { printf("rotation board failed\n"); return(errorblock); }
  printf("COUNTERCLOCKWISE\n");
  printboard(g,1);
  if( checkcell(g, 0, 4) || checkcell(g, 4, 2) || checkcell(g, 5, 7)) {
    printf("Incorrect.\n");  return(errorblock); 
  }
  freegrid(g);

  g = rotationtestgrid();
  if(!g) { printf("rotation board create failed\n"); return(errorblock); }
  rc = rotategrid(g, FLIP_TRANSPOSE);
  if(rc) { printf("rotation board failed\n"); return(errorblock); }
  printf("FLIP_TRANSPOSE\n");
  printboard(g,1);
  if( checkcell(g, 0, 0) || checkcell(g, 4, 2) || checkcell(g, 5, 7)) {
    printf("Incorrect.\n");  return(errorblock); 
  }
  freegrid(g);


  return(0);
}

