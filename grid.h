/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* tools for a maze grid */

#ifndef _GRID_H
#define _GRID_H

#define DIRECTIONS      6
#define FOURDIRECTIONS  4	/* without up / down */
#define FIRSTDIR        0
#define NORTH           0
#define WEST            1
#define EAST            2
#define SOUTH           3

/* for future work */
#define UP              4
#define DOWN            5

/* NC, no connect, is used in returns and calls.
 * others are used in calls only
 */
#define NC              -1
#define SYMMETRICAL     -2
#define ANYDIR          -3
#define ANY             -3
#define EITHER          -4
#define THIS            -5
#define EXITS		-6
#define NEIGHBORS	-7
#define OF_TYPE		-8

/* the supported rotations */
#define ROTATE_CW         90
#define CLOCKWISE         ROTATE_CW
#define ROTATE_CCW        -90
#define ROTATE_CCW_ALT    270
#define COUNTERCLOCKWISE  ROTATE_CCW
#define ROTATE_180        180
#define FLIP_LEFTRIGHT    12
#define FLIP_TOPBOTTOM    21
#define FLIP_TRANSPOSE    -78	/* transpose is same as both CCW and L-R */

/* for use with the edgestatus functions */
#define NORTH_EDGE              0x00001
#define WEST_EDGE               0x00002
#define EAST_EDGE               0x00004
#define SOUTH_EDGE              0x00008
#define NO_EDGES                0x00100
#define NO_WALLS		0x00200
#define NO_EXITS		0x00400
#define EDGE_ERROR              0x00800
#define NORTH_EXIT		0x01000
#define WEST_EXIT		0x02000
#define EAST_EXIT		0x04000
#define SOUTH_EXIT		0x08000
#define UP_EXIT   		0x10000
#define DOWN_EXIT		0x20000

#define NORTHWEST_CORNER        (NORTH_EDGE|WEST_EDGE)
#define NORTHEAST_CORNER        (NORTH_EDGE|EAST_EDGE)
#define SOUTHWEST_CORNER        (SOUTH_EDGE|WEST_EDGE)
#define SOUTHEAST_CORNER        (SOUTH_EDGE|EAST_EDGE)

/* for use with the wallstatus functions */
/* (wall|edge) gives all blockages.      */
#define NORTH_WALL	NORTH_EDGE
#define WEST_WALL	WEST_EDGE
#define EAST_WALL	EAST_EDGE
#define SOUTH_WALL	SOUTH_EDGE
#define WALL_ERROR	EDGE_ERROR
#define EXIT_ERROR	EDGE_ERROR

/* for ascii_grid() */
#define PLAIN_ASCII     0
#define USE_NAMES       1

/* Max size of a name */
#ifndef BUFSIZ
#  define BUFSIZ 1024	/* typical value from stdio.h */
#endif

/* CELLs and GRIDs have "user use" variables that can be changed at
 * will by maze makers, and other variables that should be considered
 * read only. The cell type value is used by a number of mazes.c
 * routines, most often looking for VISITED / UNVISITED (defined in mazes.h)
 * cells.
 */
typedef struct
{
   int id; /* unique id of a cell (sequential numbers) */
   int row,col; /* co-ordinates of a cell */
   /* dir array has id value of a connected cell,
    * or -1 for no connection
    */
   int dir[DIRECTIONS];

   int ctype;	/* for user use, initialized to gtype */
   char *name;	/* for user use */
   void *data;	/* for user use to hold arbitrary structures */
} CELL;

typedef struct
{
   /* total rows / columns / planes */
   int rows;
   int cols;
   int planes;
   int max;	/* size of one plane (rows*cols) */

   int gtype;	/* for user use, set at creation time */
   char *name;	/* for user use */
   void *data;	/* for user use to hold arbitrary structures */

   CELL *cells;
} GRID;

typedef struct cellcopyconfig_s {
  int origwidth;
  int newwidth;
  int rowoffset;
  int coloffset;
  int includeuserdata;
} CELLCOPYCONFIG;

void initcell(CELL*, int /*ctype*/, int /*i*/, int/*j*/, int /*id*/);
void freecell(CELL*);
int copycell(CELL *, CELL *, CELLCOPYCONFIG *);

GRID *creategrid(int /*rows*/, int /*cols*/, int /*gtype*/);
void freegrid(GRID *);
int rotategrid(GRID *, int /*rotation flag*/);
GRID *copygrid(GRID *, int /* includeuserdata */);
int pasteintogrid(GRID * /* src */, GRID * /* dest */,
	int /* top */, int /* left */, int /* includeuserdata */);
GRID *labyrinthgrid(GRID *, int /* entranceid */);

/* visit functions return a CELL pointer */
/* visitid() is the fastest of the lot */
/* macro versions should be faster than function versions */
#ifdef VISIT_FUNCTIONS
CELL *visitrc(GRID *, int /*rows*/, int /*cols*/);
CELL *visitid(GRID *, int /*cellid*/);
#else  /* VISIT_FUNCTIONS */
static int macro_id,macro_i,macro_j;
static GRID *macro_g;
#define visitrc(g,i,j)  ( macro_g=(g), macro_i = (i), macro_j = (j),                 \
                          (!macro_g || (macro_i < 0) || (macro_i >= macro_g->rows)   \
                                    || (macro_j < 0) || (macro_j >= macro_g->cols))  \
			  ?   (CELL*)NULL                                            \
			  :   &(macro_g->cells[macro_g->cols*macro_i+macro_j])       \
			)

#define visitid(g,id)   ( macro_g=(g), macro_id = (id),         \
                          (!g || (id < 0) || (id >= g->max) )   \
				? (CELL*)NULL                   \
				: &(g->cells[id])               \
			)
#endif
CELL *visitdir(GRID *, CELL */*cell*/, int/*direction*/, int/* connection status */);
CELL *visitrandom(GRID *);


/* bycell functions use one or two CELL pointers
 * byrc functions take GRID and one or two pairs of row,col
 * byid functions take GRID and one or two ids
 *
 * connect makes connections
 * isconnected tests connections
 * disconnect removes connections
 * edgestatus returns information about edges
 * wallstatus returns information about walls
 * exitstatus returns information about exits
 * natdirection returns a direction between cells (but not for any pair)
 * ncount returns a count of neighbors for a cell
 */
void connectbycell(CELL *, int /* cell1 -> cell2 direction */,
             CELL *, int /* cell2 -> cell1 direction */);

void connectbyrc(GRID *,
		int /*r1*/, int /*c1*/, int /* cell1 -> cell2 direction */,
             	int /*r2*/, int /*c2*/, int /* cell2 -> cell1 direction */);

void connectbyid(GRID *,
		int /*id1*/, int /* cell1 -> cell2 direction */,
             	int /*id2*/, int /* cell2 -> cell1 direction */);

void disconnectbycell(CELL *, int /* cell1 -> cell2 direction */,
             CELL *, int /* cell2 -> cell1 direction */);

void disconnectbyrc(GRID *,
		int /*r1*/, int /*c1*/, int /* cell1 -> cell2 direction */,
             	int /*r2*/, int /*c2*/, int /* cell2 -> cell1 direction */);

void disconnectbyid(GRID *,
		int /*id1*/, int /* cell1 -> cell2 direction */,
             	int /*id2*/, int /* cell2 -> cell1 direction */);

void disconnectbycell(CELL *, int /* cell1 -> cell2 direction */,
             CELL *, int /* cell2 -> cell1 direction */);

void disconnectbyrc(GRID *,
		int /*r1*/, int /*c1*/, int /* cell1 -> cell2 direction */,
             	int /*r2*/, int /*c2*/, int /* cell2 -> cell1 direction */);

void disconnectbyid(GRID *,
		int /*id1*/, int /* cell1 -> cell2 direction */,
             	int /*id2*/, int /* cell2 -> cell1 direction */);

int isconnectedbycell(CELL *, CELL *, int /* direction */);
int isconnectedbyrc(GRID *,
		int /*r1*/, int /*c1*/,
		int /*r2*/, int /*c2*/, int /* direction */);
int isconnectedbyid(GRID *, int /*id1*/, int /*id1*/, int /* direction */);

int edgestatusbycell(GRID *, CELL *);
int edgestatusbyrc(GRID *, int /*row*/, int/*col*/);
int edgestatusbyid(GRID *, int /*id*/);

int natdirectionbycell(CELL *, CELL *);
int natdirectionbyrc(GRID *, int /*r1*/, int /*c1*/, int /*r2*/, int /*c2*/);
int natdirectionbyid(GRID *, int /*id1*/, int /*id1*/);

int ncountbycell(GRID *, CELL *, int /*counting concern*/, int /*ctype*/);
int ncountbyrc(GRID *, int /*row*/, int/*col*/,
			int /*counting concern*/, int /*ctype*/);
int ncountbyid(GRID *, int /*id*/, int /*counting concern*/, int /*ctype*/);

int wallstatusbycell(CELL *);
int wallstatusbyrc(GRID *, int /*row*/, int/*col*/);
int wallstatusbyid(GRID *, int /*id*/);

int exitstatusbycell(CELL *);
int exitstatusbyrc(GRID *, int /*row*/, int/*col*/);
int exitstatusbyid(GRID *, int /*id*/);

/* assign a name (to a newly malloced string) to a cell */
int namebycell(CELL *, char *);
int namebyrc(GRID *, int /*row*/, int/*col*/, char *);
int namebyid(GRID *, int /*id*/, char *);

/* assign a name (to a newly malloced string) to a grid */
int namegrid(GRID *, char *);

/* call one function on each cell in a row, col, or grid.
 * that one function takes a pointer to the grid, a CELL, and
 * a pointer to a custom structure if it needs to store state.
 */
typedef int(*IFUNC_P)(GRID *,CELL*,void *);
int iteraterow(GRID *, int, IFUNC_P, void *);
int iteratecol(GRID *, int, IFUNC_P, void *);
int iterategrid(GRID *, IFUNC_P, void *);

/* find the opposite of a direction */
int opposite( int /*dir*/);

/* turn a direction into a name (don't confuse with dirname() for file
 * name manipulation)
 */
const char *dirtoname(int /*dir*/);

/* naive ascii art version of a grid */
char *ascii_grid(GRID *, int /* use_name */);


#endif
