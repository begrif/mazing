/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* test making a maze grid into an image */
/* uses libpng.h */

/* get us some strnlen */
#define _POSIX_C_SOURCE  200809L

#include "mazeimg.h"
#include "mazes.h"

#define FILENAME_SIZE	80
int
main()
{
  GRID *g;
  CELL *c;
  DMAP *dm;
  MAZEBITMAP *mb;
  int rc, errorgroup = 1, times;
  char fname[FILENAME_SIZE];
  int usecolor;
  int usedepth;

  for(times = 0; times < 11; times ++) {
    switch (times) {
       case 0: usecolor = COLOR_G;
	       usedepth = 1;
	       snprintf(fname, FILENAME_SIZE, "tmp-gray-1.png");
	       break;
       case 1: usecolor = COLOR_G;
	       usedepth = 2;
	       snprintf(fname, FILENAME_SIZE, "tmp-gray-2.png");
	       break;
       case 2: usecolor = COLOR_G;
	       usedepth = 4;
	       snprintf(fname, FILENAME_SIZE, "tmp-gray-4.png");
	       break;
       case 3: usecolor = COLOR_G;
	       usedepth = 8;
	       snprintf(fname, FILENAME_SIZE, "tmp-gray-8.png");
	       break;
       case 4: usecolor = COLOR_G;
	       usedepth = 16;
	       snprintf(fname, FILENAME_SIZE, "tmp-gray-16.png");
	       break;
       case 5: usecolor = COLOR_GA;
	       usedepth = 8;
	       snprintf(fname, FILENAME_SIZE, "tmp-grayalpha-8.png");
	       break;
       case 6: usecolor = COLOR_GA;
	       usedepth = 16;
	       snprintf(fname, FILENAME_SIZE, "tmp-grayalpha-16.png");
	       break;
       case 7: usecolor = COLOR_RGB;
	       usedepth = 8;
	       snprintf(fname, FILENAME_SIZE, "tmp-rgb-8.png");
	       break;
       case 8: usecolor = COLOR_RGB;
	       usedepth = 16;
	       snprintf(fname, FILENAME_SIZE, "tmp-rgb-16.png");
	       break;
       case 9: usecolor = COLOR_RGBA;
	       usedepth = 8;
	       snprintf(fname, FILENAME_SIZE, "tmp-rgbalpha-8.png");
	       break;
       case 10: usecolor = COLOR_RGBA;
	       usedepth = 16;
	       snprintf(fname, FILENAME_SIZE, "tmp-rgbalpha-16.png");
	       break;
    }
    
    g = creategrid(16,16,1);
    if(!g) {
      printf("Create serpentine grid failed.\n");
      return errorgroup;
    }

    iterategrid(g, serpentine, NULL);
    namebyid(g, g->rows - 1, " A");
    namebyid(g, g->max  - 1, " B");
    dm = createdistancemap(g, visitid(g, g->rows - 1) );
    if(!dm) {
      printf("Create distancemap failed.\n");
      return errorgroup;
    }
    /* non-lazy distance map */
    distanceto(dm, visitid(g, g->max  - 1), 0);

    rc = findpath(dm);
    if( rc != 0 ) {
      printf("Find path failed %d\n", rc);
      return errorgroup;
    }

    mb = createmazebitmap(dm);
    if(!mb) {
      printf("Create mazebitmap failed.\n");
      return errorgroup;
    }

    rc = initmazebitmap(mb, 15, 15, usecolor, usedepth, CELL_SIZE);
    if( rc < 0 ) {
      printf("init mazebitmap failed %d\n", rc);
      return errorgroup;
    } else if (rc == 0){
      printf("init mazebitmap returned 0, good for only PNM\n");
    } else if (rc == 1){
      printf("init mazebitmap returned 1, suitable for all images\n");
    } else {
      printf("init mazebitmap returned %d\n", rc);
    }

    rc = drawmaze(mb);
    printf("drawmaze returned %d, ", rc);
    if( rc == g->max ) {
      printf("as expected\n");
    } else {
      printf("which is wrong\n");
      return errorgroup;
    }

    rc = writepng(mb, fname);
    if(!rc) {
      printf("Wrote %s\n",fname);
    }

    /* suffix tweaking: .png to .pnm */
    int j = strnlen(fname, FILENAME_SIZE);
    j -= 1;
    if(j > 4) { /* safety check on short file names */
      fname[j] = 'm';
      rc = writepnm(mb, fname);
      if(!rc) {
        printf("Wrote %s\n",fname);
      }
    }
    freemazebitmap(mb);
    freedistancemap(dm);
    freegrid(g);

    errorgroup ++;
  } /* looping over all formats */

  return 0;
}
