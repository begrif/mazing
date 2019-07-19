/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* testing maze to labyrinth (and exitstatus()) */

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
printoneexit(int cid, char dir, int status)
{
  printf(" %c: ", dir);
  if(status && (cid == NC)) {
    printf(" -- (WRONG) ");
    return 1;
  }
  if(cid == NC) {
    printf(" -- (RIGHT) ");
    return 0;
  }
  if(status) {
    printf(" %d (RIGHT) ", cid);
    return 0;
  }
  printf(" %d (WRONG) ", cid);
  return 1;
}

int
printexits(CELL *c)
{
  char *dir;
  int rc = 0;
  int exits = exitstatusbycell(c);

  if(!c) {
    printf("printexits(c): not a valid cell\n");
    return 5;
  }
  printf("Cell id %d (at %d,%d) ", c->id, c->row, c->col);

  rc += printoneexit(c->dir[NORTH], 'N', (exits & NORTH_EXIT));
  rc += printoneexit(c->dir[SOUTH], 'S', (exits & SOUTH_EXIT));
  rc += printoneexit(c->dir[WEST] , 'W', (exits & WEST_EXIT));
  rc += printoneexit(c->dir[EAST] , 'E', (exits & EAST_EXIT));
  printf("\n");

  return rc;
}

int
main(int ignored, char**notused)
{
  GRID *g, *little;
  CELL *cs, *c1, *c2;
  int rc, errorblock = 1;
  char *board;
  const char expected0[] =
	  "+---+---+---+---+\n"
	  "|   |   |       |\n"
	  "+---+---+   +   +\n"
	  "|   |   |   |   |\n"
	  "+---+---+   +   +\n"
	  "|           |   |\n"
	  "+   +---+---+   +\n"
	  "|               |\n"
	  "+---+---+---+---+\n"
  ;
  const char expected1[] = 
	  "+---+---+---+---+\n"
	  "|       |       |\n"
	  "+   +   +   +   +\n"
	  "|   |   |   |   |\n"
	  "+   +   +   +   +\n"
	  "|   |       |   |\n"
	  "+   +---+---+   +\n"
	  "|   |       |   |\n"
	  "+   +   +   +   +\n"
	  "|   |   |   |   |\n"
	  "+   +   +   +   +\n"
	  "|       |   |   |\n"
	  "+---+---+---+---+\n"
  ;
  const char expected2[] =
	  "+---+---+---+---+---+---+\n"
	  "|               |       |\n"
	  "+   +---+---+   +   +   +\n"
	  "|   |       |   |   |   |\n"
	  "+   +   +   +   +   +   +\n"
	  "|   |   |   |   |   |   |\n"
	  "+   +   +   +   +   +---+\n"
	  "|   |   |   |   |   |   |\n"
	  "+   +   +   +   +   +   +\n"
	  "|   |   |   |       |   |\n"
	  "+   +   +   +---+---+   +\n"
	  "|       |               |\n"
	  "+---+---+---+---+---+---+\n"
  ;

  little = creategrid(3,1,5);
  if(!little) { 
    printf("labyrinth source grid failed\n");
    return errorblock;
  }
  connectbyid(little,   0,    SOUTH,     1, SYMMETRICAL);
  connectbyid(little,   1,    SOUTH,     2, SYMMETRICAL);

  printf("\nSource for labyrinth\n");
  printboard(little, 0);

  g = labyrinthgrid(little, NC);
  if(!g) {
    printf("labyrinth build grid failed\n");
    return errorblock;
  }
  printboard(g, 0);

  printf("Original top cell in labyrinth\n");
  printf("(RIGHT/WRONG only testing exitstatusbycell())\n");
  rc  = printexits(visitrc(g, 0, 0));
  rc += printexits(visitrc(g, 0, 1));
  rc += printexits(visitrc(g, 1, 0));
  rc += printexits(visitrc(g, 1, 1));
  if(rc) {
    return errorblock;
  }

  printf("Original middle cell in labyrinth\n");
  rc  = printexits(visitrc(g, 2, 0));
  rc += printexits(visitrc(g, 2, 1));
  rc += printexits(visitrc(g, 3, 0));
  rc += printexits(visitrc(g, 3, 1));
  if(rc) {
    return errorblock;
  }
  printf("Original bottom cell in labyrinth\n");
  rc  = printexits(visitrc(g, 4, 0));
  rc += printexits(visitrc(g, 4, 1));
  rc += printexits(visitrc(g, 5, 0));
  rc += printexits(visitrc(g, 5, 1));
  if(rc) {
    return errorblock;
  }

  freegrid(little);
  freegrid(g);
  errorblock ++;

  little = creategrid(1,3,5);
  if(!little) { 
    printf("labyrinth source grid failed\n");
    return errorblock;
  }
  connectbyid(little,   0,    EAST,     1, SYMMETRICAL);
  connectbyid(little,   1,    EAST,     2, SYMMETRICAL);

  printf("Source for labyrinth\n");
  printboard(little, 0);

  g = labyrinthgrid(little, NC);
  if(!g) {
    printf("labyrinth build grid failed\n");
    return errorblock;
  }
  printboard(g, 0);

  printf("Original left cell in labyrinth\n");
  rc  = printexits(visitrc(g, 0, 0));
  rc += printexits(visitrc(g, 1, 0));
  rc += printexits(visitrc(g, 0, 1));
  rc += printexits(visitrc(g, 1, 1));
  if(rc) {
    return errorblock;
  }

  printf("Original middle cell in labyrinth\n");
  rc  = printexits(visitrc(g, 0, 2));
  rc += printexits(visitrc(g, 1, 2));
  rc += printexits(visitrc(g, 0, 3));
  rc += printexits(visitrc(g, 1, 3));
  if(rc) {
    return errorblock;
  }
  printf("Original right cell in labyrinth\n");
  rc  = printexits(visitrc(g, 0, 4));
  rc += printexits(visitrc(g, 1, 4));
  rc += printexits(visitrc(g, 0, 5));
  rc += printexits(visitrc(g, 1, 5));
  if(rc) {
    return errorblock;
  }

  freegrid(little);
  freegrid(g);
  errorblock ++;

  printf("\nNow testing off-labyrinth exits\n");
  little = creategrid(1,1,1);
  if(!little) { 
    printf("labyrinth source grid failed\n");
    return errorblock;
  }
  cs = visitid(little, 0);
  if(!cs) {
    printf("visit labyrinth source cell failed\n");
    return errorblock;
  }
  cs->dir[NORTH] = 51; /* way off-grid */
  g = labyrinthgrid(little, cs->id);
  if(!g) {
    printf("labyrinth build grid failed\n");
    return errorblock;
  }
  c1 = visitrc(g, 0, 0);  
  c2 = visitrc(g, 0, 1);  
  if(!c1 || !c2) {
    printf("labyrinth cell visit failed\n");
    return errorblock;
  }
  if(c1->dir[NORTH] != cs->dir[NORTH]) {
    printf("labyrinth cell id %d not properly linked\n", c1->id);
    printf("is %d not to %d\n", c1->dir[NORTH], cs->dir[NORTH]);
    return errorblock;
  }
  if(c2->dir[NORTH] != cs->dir[NORTH]) {
    printf("labyrinth cell id %d not properly linked\n", c2->id);
    printf("is %d not to %d\n", c2->dir[NORTH], cs->dir[NORTH]);
    return errorblock;
  }

  printf("NORTH passes\n");
  freegrid(little);
  freegrid(g);
  errorblock ++;

  little = creategrid(1,1,1);
  if(!little) { 
    printf("labyrinth source grid failed\n");
    return errorblock;
  }
  cs = visitid(little, 0);
  if(!cs) {
    printf("visit labyrinth source cell failed\n");
    return errorblock;
  }
  cs->dir[SOUTH] = 85; /* way off-grid */
  g = labyrinthgrid(little, cs->id);
  if(!g) {
    printf("labyrinth build grid failed\n");
    return errorblock;
  }
  c1 = visitrc(g, 1, 0);  
  c2 = visitrc(g, 1, 1);  
  if(!c1 || !c2) {
    printf("labyrinth cell visit failed\n");
    return errorblock;
  }
  if(c1->dir[SOUTH] != cs->dir[SOUTH]) {
    printf("labyrinth cell id %d not properly linked\n", c1->id);
    printf("is %d not to %d\n", c1->dir[SOUTH], cs->dir[SOUTH]);
    return errorblock;
  }
  if(c2->dir[SOUTH] != cs->dir[SOUTH]) {
    printf("labyrinth cell id %d not properly linked\n", c2->id);
    printf("is %d not to %d\n", c2->dir[SOUTH], cs->dir[SOUTH]);
    return errorblock;
  }

  printf("SOUTH passes\n");
  freegrid(little);
  freegrid(g);
  errorblock ++;

  little = creategrid(1,1,1);
  if(!little) { 
    printf("labyrinth source grid failed\n");
    return errorblock;
  }
  cs = visitid(little, 0);
  if(!cs) {
    printf("visit labyrinth source cell failed\n");
    return errorblock;
  }
  cs->dir[EAST] = -17; /* way off-grid */
  g = labyrinthgrid(little, cs->id);
  if(!g) {
    printf("labyrinth build grid failed\n");
    return errorblock;
  }
  c1 = visitrc(g, 0, 1);  
  c2 = visitrc(g, 1, 1);  
  if(!c1 || !c2) {
    printf("labyrinth cell visit failed\n");
    return errorblock;
  }
  if(c1->dir[EAST] != cs->dir[EAST]) {
    printf("labyrinth cell id %d not properly linked\n", c1->id);
    printf("is %d not to %d\n", c1->dir[EAST], cs->dir[EAST]);
    return errorblock;
  }
  if(c2->dir[EAST] != cs->dir[EAST]) {
    printf("labyrinth cell id %d not properly linked\n", c2->id);
    printf("is %d not to %d\n", c2->dir[EAST], cs->dir[EAST]);
    return errorblock;
  }

  printf("EAST passes\n");
  freegrid(little);
  freegrid(g);
  errorblock ++;

  printf("Now for a self-link version\n");

  little = creategrid(1,1,1);
  if(!little) { 
    printf("labyrinth source grid failed\n");
    return errorblock;
  }
  cs = visitid(little, 0);
  if(!cs) {
    printf("visit labyrinth source cell failed\n");
    return errorblock;
  }
  cs->dir[WEST] = cs->id; /* link to self */
  g = labyrinthgrid(little, cs->id);
  if(!g) {
    printf("labyrinth build grid failed\n");
    return errorblock;
  }
  c1 = visitrc(g, 0, 0);  
  c2 = visitrc(g, 1, 0);  
  if(!c1 || !c2) {
    printf("labyrinth cell visit failed\n");
    return errorblock;
  }
  if(c1->dir[WEST] != c1->id) {
    printf("labyrinth cell id %d not properly linked\n", c1->id);
    printf("is %d not to %d\n", c1->dir[WEST], c1->id);
    return errorblock;
  }
  if(c2->dir[WEST] != c2->id) {
    printf("labyrinth cell id %d not properly linked\n", c2->id);
    printf("is %d not to %d\n", c2->dir[WEST], c2->id);
    return errorblock;
  }

  printf("WEST passes\n");
  freegrid(little);
  freegrid(g);
  errorblock ++;

  printf("\nTesting ASCII images (note off-grid entrances show as walls)\n");
  little = creategrid(2,2,8);
  if(!little) { 
    printf("labyrinth source grid failed\n");
    return errorblock;
  }
  connectbyid(little,   2,    EAST,     3, SYMMETRICAL);
  connectbyid(little,   1,   SOUTH,     3, SYMMETRICAL);
  namebyid(   little,   0,  "n/a");

  printf("Source for labyrinth\n");
  printboard(little, 1);

  g = labyrinthgrid(little, NC);
  if(!g) {
    printf("labyrinth build grid failed\n");
    return errorblock;
  }
  board = ascii_grid(g, 0);
  if(0 != strncmp(expected0, board, BUFSIZ)) {
    printf("Expected:\n%s", expected0);
    printf("Got:\n%s", board);
    return errorblock; 
  }
  printf("As expected:\n%s", board);
  free(board);


  freegrid(little);
  freegrid(g);
  errorblock ++;

  little = creategrid(3,2,5);
  if(!little) { 
    printf("labyrinth source grid failed\n");
    return errorblock;
  }
  connectbyid(little,   2,    EAST,     3, SYMMETRICAL);
  connectbyid(little,   0,   SOUTH,     2, SYMMETRICAL);
  connectbyid(little,   1,   SOUTH,     3, SYMMETRICAL);
  connectbyid(little,   2,   SOUTH,     4, SYMMETRICAL);
  connectbyid(little,   3,   SOUTH,     5, SYMMETRICAL);
  connectbyid(little,   5,   SOUTH,     5, SOUTH);
  namebyid(   little,   5,  "entrance");

  printf("\nSource for labyrinth\n");
  printboard(little, 1);

  g = labyrinthgrid(little, NC);
  if(!g) {
    printf("labyrinth build grid failed\n");
    return errorblock;
  }
  board = ascii_grid(g, 0);
  if(0 != strncmp(expected1, board, BUFSIZ)) {
    printf("Expected:\n%s", expected1);
    printf("Got:\n%s", board);
    return errorblock; 
  }
  printf("As expected:\n%s", board);
  free(board);


  freegrid(little);
  freegrid(g);
  errorblock ++;

  little = creategrid(3,3,5);
  if(!little) { 
    printf("labyrinth source grid failed\n");
    return errorblock;
  }
  connectbyid(little,   0,    EAST,     1, SYMMETRICAL);
  connectbyid(little,   7,    EAST,     8, SYMMETRICAL);
  connectbyid(little,   0,   SOUTH,     3, SYMMETRICAL);
  connectbyid(little,   1,   SOUTH,     4, SYMMETRICAL);
  connectbyid(little,   2,   SOUTH,     5, SYMMETRICAL);
  connectbyid(little,   3,   SOUTH,     6, SYMMETRICAL);
  connectbyid(little,   4,   SOUTH,     7, SYMMETRICAL);
  connectbyid(little,   5,   SOUTH,     8, SYMMETRICAL);
  connectbyid(little,   5,    EAST,     5, EAST);
  namebyid(   little,   5,  "entrance");

  printf("\nSource for labyrinth\n");
  printboard(little, 1);

  g = labyrinthgrid(little, NC);
  if(!g) {
    printf("labyrinth build grid failed\n");
    return errorblock;
  }
  board = ascii_grid(g, 0);
  if(0 != strncmp(expected2, board, BUFSIZ)) {
    printf("Expected:\n%s", expected2);
    printf("Got:\n%s", board);
    return errorblock; 
  }
  printf("As expected:\n%s", board);
  free(board);

  freegrid(little);
  freegrid(g);
  errorblock ++;

  return 0;
}
