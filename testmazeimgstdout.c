/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* test making a maze grid into an image (writing to stdout) */
/* uses libpng.h */

#include <stdio.h>
#include <stdlib.h>

#include "mazeimg.h"
#include "mazes.h"

/* example 7x7 cell, E edge color, W wall call, B background, F foreground
 * this is a left edge cell with a wall to north (so exits east and south)
 *
 *   EBBBBBB    <- long background
 *   EWWWWWW    <- long walls
 *   EWFFFFF
 *   EWFFFFF
 *   EWFFFFF
 *   EWFFFWW    <- short walls (only for exits on two adjacent walls)
 *   EWFFFWB    <- pin dot background
 *   ^
 *   long edge
 *
 * This is a 3 wide path with 1 wide walls on either side and background
 * or edge outside the walls.
 */ 
int 
drawcell7x7(MAZEBITMAP *mb, png_byte *image, CELL *c)
{
  int walls, edges, i1, i2, j1, j2;
  int siz = mb->channels;
  int twofer = mb->doubled + 1;
  int wid = mb->cell_w;
  int hei = mb->cell_h;
  int is_a, is_b;
  COLORDATA *colors;

  colors = (COLORDATA *)mb->udata;
  is_a = is_b = 0;

  if (c->name && (c->name[0] == 'A')) {
    /* start of path */
    fill_cell(image, wid * hei, siz, twofer, &(colors->uc1[0]));
    /* only use edge knockout if a/b are where expected */
    if(c->id == 0) { is_a = 1; }
  }

  else if (c->name && (c->name[0] == 'B')) {
    /* end of path */
    fill_cell(image, wid * hei, siz, twofer, &(colors->uc2[0]));
    /* only use edge knockout if a/b are where expected */
    if(c->id == mb->dmap->grid->max - 1) { is_b = 1; }
  }

  else if (c->name && (c->name[0] == 'P')) {
    /* on the path */
    fill_cell(image, wid * hei, siz, twofer, &(colors->uc4[0]));
  }
  
  else {
    fill_cell(image, wid * hei, siz, twofer, &(colors->fg[0]));
  }

  walls = wallstatusbycell(c);
  edges = edgestatusbycell(mb->dmap->grid, c);

  /* i1 and i2 will be 0 or (hei - 1) for all backgrounds and edges
   *           will be 0, 1 (hei - 1), or (hei - 2) for all walls
   * j1 and j2 will be 0 or (wid - 1) for all backgrounds and edges
   *           will be 0, 1 or (hei - 1), (wid - 2) for all walls
   */

  /* long backgrounds and long edges */
  if(!is_a) {
    i1 = i2 = 0;
    j1 = 0;
    j2 = wid - 1;
    if(walls & NORTH_WALL) {
      draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors->bg[0]));
    }
    if(edges & NORTH_EDGE) {
      draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors->edge[0]));
    }
  }

  if(!is_b) {
    i1 = i2 = hei - 1;
    j1 = 0;
    j2 = wid - 1;
    if(walls & SOUTH_WALL) {
      draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors->bg[0]));
    }
    if(edges & SOUTH_EDGE) {
      draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors->edge[0]));
    }
  }

  if(!is_a) {
    j1 = j2 = 0;
    i1 = 0;
    i2 = hei - 1;
    if(walls & WEST_WALL) {
      if(edges & NORTH_EDGE) { i1 ++; }
      if(edges & SOUTH_EDGE) { i2 --; }
      draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors->bg[0]));
    }
    if(edges & WEST_EDGE) {
      i1 = 0;
      i2 = hei - 1;
      draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors->edge[0]));
    }
  }

  if(!is_b) {
    j1 = j2 = wid - 1;
    i1 = 0;
    i2 = hei - 1;
    if(walls & EAST_WALL) {
      if(edges & NORTH_EDGE) { i1 ++; }
      if(edges & SOUTH_EDGE) { i2 --; }
      draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors->bg[0]));
    }
    if(edges & EAST_EDGE) {
      i1 = 0;
      i2 = hei - 1;
      draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors->edge[0]));
    }
  }

  /* long walls */
  if(!is_a) {
    if(walls & NORTH_WALL) {
      i1 = i2 = 1;
      if(walls & WEST_WALL) { j1 = 1;       } else { j1 = 0; }
      if(walls & EAST_WALL) { j2 = wid - 2; } else { j2 = wid - 1; }
      draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors->wall[0]));
    }
  }

  if(!is_b) {
    if(walls & SOUTH_WALL) {
      i1 = i2 = hei - 2;
      if(walls & WEST_WALL) { j1 = 1;       } else { j1 = 0; }
      if(walls & EAST_WALL) { j2 = wid - 2; } else { j2 = wid - 1; }
      draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors->wall[0]));
    }
  }

  if(!is_a) {
    if(walls & WEST_WALL) {
      j1 = j2 = 1;
      if(walls & NORTH_WALL) { i1 = 1;       } else { i1 = 0; }
      if(walls & SOUTH_WALL) { i2 = hei - 2; } else { i2 = hei - 1; }
      draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors->wall[0]));
    }
  }

  if(!is_b) {
    if(walls & EAST_WALL) {
      j1 = j2 = wid - 2;
      if(walls & NORTH_WALL) { i1 = 1;       } else { i1 = 0; }
      if(walls & SOUTH_WALL) { i2 = hei - 2; } else { i2 = hei - 1; }
      draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors->wall[0]));
    }
  }

  /* short walls and pin backgrounds */
  if( !(walls & NORTH_WALL) && !(walls & EAST_WALL) ) { /* NE corner */
    i1 = i2 = 1;
    j1 = wid - 2;
    j2 = wid - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors->wall[0]));
    i1 --;
    /* note j1 used twice for east, then j2 twice */
    draw_a_line(image, i1, j1, i2, j1, wid, siz, twofer, &(colors->wall[0]));
    draw_a_line(image, i1, j2, i1, j2, wid, siz, twofer, &(colors->bg[0]));
  }

  if( !(walls & SOUTH_WALL) && !(walls & EAST_WALL) ) { /* SE corner */
    i1 = i2 = hei - 2;
    j1 = wid - 2;
    j2 = wid - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors->wall[0]));
    i2 ++;
    /* note j1 used twice for east, then j2 twice */
    draw_a_line(image, i1, j1, i2, j1, wid, siz, twofer, &(colors->wall[0]));
    draw_a_line(image, i2, j2, i2, j2, wid, siz, twofer, &(colors->bg[0]));
  }

  if( !(walls & SOUTH_WALL) && !(walls & WEST_WALL) ) { /* SW corner */
    i1 = i2 = hei - 2;
    j1 = 0;
    j2 = 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors->wall[0]));
    i2 ++;
    /* note j2 used twice for west, then j1 twice */
    draw_a_line(image, i1, j2, i2, j2, wid, siz, twofer, &(colors->wall[0]));
    draw_a_line(image, i2, j1, i2, j1, wid, siz, twofer, &(colors->bg[0]));
  }

  if( !(walls & NORTH_WALL) && !(walls & WEST_WALL) ) { /* NW corner */
    i1 = i2 = 1;
    j1 = 0;
    j2 = 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors->wall[0]));
    i1 --;
    /* note j2 used twice for west then j1 twice */
    draw_a_line(image, i1, j2, i2, j2, wid, siz, twofer, &(colors->wall[0]));
    draw_a_line(image, i1, j1, i1, j1, wid, siz, twofer, &(colors->bg[0]));
  }


  return 1;
} /* drawcell7x7 */

int
main()
{
  GRID *g;
  CELL *c;
  DMAP *dm;
  MAZEBITMAP *mb;
  int cell_a, cell_b;
  int grid_size, image_size, depth, color;
  int rc, errorgroup = 1, times;

  grid_size = 75;
  image_size = 7 * grid_size;
  color = COLOR_RGB;
  depth = 8;

  g = creategrid(grid_size, grid_size, UNVISITED);
  if(!g) {
    fprintf(stderr, "Create big grid failed.\n");
    return errorgroup;
  }

  srandom(34);		/* for a reproducible random number sequence */
  rc = backtracker(g, 0);
  if(rc) {
    fprintf(stderr, "Failed to draw a backtrack maze\n");
  }

  cell_a = 0;
  cell_b = g->max - 1;

  dm = createdistancemap(g, visitid(g, cell_b) );
  if(!dm) {
    fprintf(stderr, "Create distancemap failed.\n");
    return errorgroup;
  }
  /* non-lazy distance map */
  distanceto(dm, visitid(g, cell_a), 0);

  rc = findpath(dm);
    if( rc != 0 ) {
    fprintf(stderr, "Find path failed %d\n", rc);
    return errorgroup;
  }
  /* It's solved backwards, so A and B are reversed */
  namepath(dm, "BBB", "PPP", "AAA");

  mb = createmazebitmap(dm);
  if(!mb) {
    fprintf(stderr, "Create mazebitmap failed.\n");
    return errorgroup;
  }

  rc = initmazebitmap(mb, image_size, image_size, color, depth, MAZE_SIZE);
  if( rc < 0 ) {
    fprintf(stderr, "init mazebitmap failed %d\n", rc);
    return errorgroup;
  }

  mb->cellfunc = (int(*)(MAZEBITMAP *, void *, CELL *))drawcell7x7;
  COLORDATA colors = {
      .depth    = mb->colordepth,
      .channels = mb->channels,
      .edge = { 0x00, 0x00, 0x00, 0 },
      .wall = { 0x24, 0x90, 0x90, 0 },
      .bg   = { 0xC0, 0xC5, 0xB9, 0 },
      .fg   = { 0xB0, 0xD0, 0x24, 0 },
      .uc1  = { 0xFF, 0xFF, 0x2A, 0 }, /* point A */
      .uc2  = { 0x2B, 0xFF, 0xFF, 0 }, /* point B */
      .uc3  = { 0xCC, 0xCC, 0xCC, 0 }, /* was used in debugging only */
      .uc4  = { 0x90, 0x24, 0x90, 0 }, /* walk */
  };
  mb->udata = &colors;

  rc = drawmaze(mb);
  fprintf(stderr, "drawmaze returned %d, ", rc);
  if( rc == g->max ) {
    fprintf(stderr, "as expected\n");
  } else {
    fprintf(stderr, "which is wrong\n");
    return errorgroup;
  }

  rc = writepnm(mb, "-");
  if(!rc) {
    fprintf(stderr, "Wrote to stdout\n");
  }

  freemazebitmap(mb);
  freedistancemap(dm);
  freegrid(g);

  return 0;
}
