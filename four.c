/* July 2019, Benjamin Elijah Griffin / Eli the Bearded */

/* Build a maze using four different algos in a tiled pattern.
 */

#ifndef _XOPEN_SOURCE
/* get us some strndup() */
# define _XOPEN_SOURCE 700
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "grid.h"
#include "distance.h"
#include "mazeimg.h"
#include "mazes.h"
#include "forfour.h"

#define DEFAULTBASE  "four-default"
#define BASEORDEFAULT(b) ( (b)? (b) : DEFAULTBASE )


/* this gets used twice: the solved and unsolved mazes */
int
drawandsave(MAZEBITMAP *mb, char*fname)
{
  int rc = drawmaze(mb);
  if(rc != mb->dmap->grid->max) {
    fprintf(stderr, "Maze draw of %s failed; that's odd.\n", fname);
    return 2;
  }

  rc = writepng(mb, fname);
  if(rc != 0) {
    fprintf(stderr, "Maze save of %s failed; that's odd.\n", fname);
    return 2;
  }

  return 0;
} /* drawandsave() */


/* all of the sub-mazes are created with this */
GRID *
createamaze(int tilesize, int mazetype)
{
  int rc;
  GRID *amaze = creategrid(tilesize, tilesize, UNVISITED);

  if(!amaze) {
    fprintf(stderr, "Create subgrid failed, probably memory.\n");
    return NULL;
  }

  switch(mazetype % 4) {
    case 0: rc = aldbro(amaze, NULL);	break;
    case 1: rc = wilson(amaze, NULL);	break;
    case 2: rc = huntandkill(amaze, NULL);	break;
    case 3: rc = backtracker(amaze, NULL);	break;
  }

  if(rc) {
    fprintf(stderr, "Maze failed (type %d)\n", mazetype);
    return NULL;
  }

  return amaze;
} /* createamate() */

int
main(int argc, char**argv)
{
  int tilesize = 5;
  int cellsize = 15;
  int showascii = 0;
  char *filename = NULL;
  char *basename = NULL;
  GRID *maing, *superg, *subg;
  CELL *superc;
  DMAP *dm;
  MAZEBITMAP *mb;
  int r,c;	/* row and column of tiles */
  int i,j;	/* row and column of cells */
  int a;	/* used during args parsing */
  int rc;	/* return code */
  int rn;	/* random number */
  int mazetype; /* what to draw in a tile */
  color_overide_t usercolors;

  usercolors[0][0] = usercolors[1][0] = usercolors[2][0] = usercolors[3][0] = 
  usercolors[4][0] = usercolors[5][0] = usercolors[6][0] = usercolors[7][0] = 0;
  a = 0;
  while( a != -1 ) {
    int i;
    static struct option opts[] = {
	 { "cellsize",  required_argument,  0,  'c' },
	 { "tilesize",  required_argument,  0,  't' },
	 { "basefile",  required_argument,  0,  'b' },
	 { "edge",      required_argument,  0,  'E' },
	 { "wall",      required_argument,  0,  'W' },
	 { "start",     required_argument,  0,  'S' },
	 { "finish",    required_argument,  0,  'F' },
	 { "bg",        required_argument,  0,  'B' },
	 { "answer",    required_argument,  0,  'A' },
	 { "showascii", no_argument,        0,  'a' },
	 { 0,0,0,0 }
      };

    a = getopt_long(argc, argv, "c:t:b:aE:W:S:F:B:A:", opts, &i);
    switch (a) {
      case 'A': case 'B': case 'F': case 'S': case 'W': case 'E':
	if(verifycolor(optarg)) {
	  copycolor(usercolors, a, optarg);
	} else {
	  fprintf(stderr, "Color %s can't be understood.\n", optarg);
	  return 1;
	}
        break;

      case 'a':
        showascii = 1;
	break;

      case 'c':
        cellsize = atoi(optarg);
	if(cellsize < 3) {
	  fprintf(stderr, "cellsize %d is too small, minimum is 3\n", cellsize);
	  return 1;
	}
        break;

      case 't':
        tilesize = atoi(optarg);
	if(tilesize < 4) {
	  fprintf(stderr, "tilesize %d is too small, minimum is 4\n", tilesize);
	  return 1;
	}
        break;

      case 'b':
        if(basename) {
	  fprintf(stderr, "Base filename specified twice\n");
	  return 1;
	}
        basename = strndup(optarg, BUFSIZ);
        break;

      case '?':
        printf("Usage: four can take three options\n");
	printf("   -c  NUM   --cellsize NUM    pixel size of a cell\n");
	printf("   -t  NUM   --tilesize NUM    tile size of a submaze\n");
	printf("   -b  NAME  --basename NAME   base filename for output\n");
	return 1;
        break;

      case -1:
      	/* end of args */
	break;

      default:
	fprintf(stderr, "Unexpected arg: a is %d\n", a);
	return 3;
    } /* switch on arg */
  } /* while arg parsing */

  if(optind < argc) {
    fprintf(stderr, "Extraneous parameter(s): '%s', etc\n", argv[optind]);
    return 1;
  }

  maing = creategrid(tilesize*4, tilesize*4, 4);
  if(!maing) {
    fprintf(stderr, "Create grid failed, probably memory.\n");
    return 2;
  }

  /* The super maze is the maze that determines how the tiles connect. */
  superg = creategrid(4, 4, 0);
  if(!superg) {
    fprintf(stderr, "Create super grid failed, probably memory.\n");
    return 2;
  }

  mazetype = random() % 4;
  superg = createamaze(4, mazetype);
  if(!superg) {
    fprintf(stderr, "Super create failed (type %d)\n", mazetype);
    return 2;
  }

  mazetype = 0;
  for(r = 0; r < 4; r ++) {
    for(c = 0; c < 4; c ++) {
      i = r * tilesize;
      j = c * tilesize;

      subg = createamaze(tilesize, mazetype);

      if(!subg) {
        fprintf(stderr, "Maze create failed (type %d at %d,%d)\n",
		mazetype, r,c);
	return 2;
      }

      rc = pasteintogrid(subg, maing, i, j, 0);
      if(rc) {
        fprintf(stderr, "Paste failed (%d,%d)\n", r,c);
	return 2;
      }
      freegrid(subg);

      superc = visitrc(superg, r, c);
      if(r != 0) {
        /* not row zero, maybe connect this tile to one above */
	if(NC != superc->dir[NORTH]) {
	  rn = random() % (tilesize - 1);
	  connectbyrc(maing, i - 1, j + rn, SOUTH, i, j + rn, NORTH);
	}
      }

      if(c != 0) {
        /* not column zero, maybe connect this tile to one to left */
	if(NC != superc->dir[WEST]) {
	  rn = random() % (tilesize - 1);
	  connectbyrc(maing, i + rn, j - 1, EAST, i + rn, j, WEST);
	}
      }

      mazetype ++;
    } /* for c */

    mazetype ++; /* stagger mazes per row */
  } /* for r */

  /* no need for this anymore */
  freegrid(superg);

  dm = createdistancemap(maing, visitid(maing, 0));
  if(!dm) {
    fprintf(stderr, "Create distance map failed, probably memory\n");
    return 2;
  }

  /* find distances to everywhere */
  rc = distanceto(dm, visitid(maing, maing->max - 1), NONLAZYMAP);

  /* solve it! */
  rc = findpath(dm);
  
  /* change celltype on solved path */
  rc = iteratewalk(dm, marksolved, NULL);

  /* create a virtual "knockdown" to enter maze */
  connectbyid(maing, 0, NORTH, 0, NORTH);

  /* and another to exit maze */
  connectbyid(maing, maing->max - 1, SOUTH, maing->max - 1, SOUTH);

  mb = createmazebitmap(dm);
  if(!mb) {
    fprintf(stderr, "Create mazebitmap failed; gonna guess memory.\n");
    return 2;
  }

  rc = initmazebitmap(mb, cellsize, cellsize, COLOR_RGB, 8, CELL_SIZE);
  /* 0 is acceptable for non-png output */
  if(rc != 1) {
    fprintf(stderr, "Init mazebitmap failed; gonna guess memory.\n");
    return 2;
  }

  if(showascii) {
    char *board = ascii_grid(mb->dmap->grid, 0);
    puts(board);
    free(board);
  }

  /* set our callback */
  mb->cellfunc  = (CELLFUNC_P) cleandraw;

  /* store our colors in the maze */
  mb->udata = colors(usercolors);

  filename = (char*) malloc( BUFSIZ );
  if(!filename) {
    fprintf(stderr, "Filename malloc failed.\n");
    return 2;
  }

  /* the regular maze */
  snprintf(filename, BUFSIZ, "%s-maze.png", BASEORDEFAULT(basename));
  rc = drawandsave(mb, filename);
  if(rc != 0) {
    /* error message already printed */
    return 2;
  }

  /* flag and draw solved maze */
  maing->gtype = SOLVEDCELL;
  snprintf(filename, BUFSIZ, "%s-answer.png", BASEORDEFAULT(basename));
  rc = drawandsave(mb, filename);
  if(rc != 0) {
    /* error message already printed */
    return 2;
  }

  free(filename);
  free(mb->udata);
  freemazebitmap(mb);
  freedistancemap(dm);
  freegrid(maing);

  return 0;
} /* main */

