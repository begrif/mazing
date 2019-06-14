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

/* for use with the edgestatus functions */
#define NORTH_EDGE              0x001
#define WEST_EDGE               0x002
#define EAST_EDGE               0x004
#define SOUTH_EDGE              0x008
#define NO_EDGES                0x100
/* used below                   0x200 */
#define EDGE_ERROR              0x800
#define NORTHWEST_CORNER        (0x001|0x002)
#define NORTHEAST_CORNER        (0x001|0x004)
#define SOUTHWEST_CORNER        (0x008|0x002)
#define SOUTHEAST_CORNER        (0x008|0x004)

/* for use with the wallstatus functions */
/* (wall|edge) gives all blockages.      */
#define NORTH_WALL	NORTH_EDGE
#define WEST_WALL	WEST_EDGE
#define EAST_WALL	EAST_EDGE
#define SOUTH_WALL	SOUTH_EDGE
#define NO_WALLS	0x200
#define WALL_ERROR	EDGE_ERROR

/* Max size of a name */
#ifndef BUFSIZ
#  define BUFSIZ 1024	/* typical value from stdio.h */
#endif

/* CELLs and GRIDs have "user use" variables that can be changed at
 * will by maze makers, and other variables that should be considered
 * read only.
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

void initcell(CELL*, int /*ctype*/, int /*i*/, int/*j*/, int /*id*/);
void freecell(CELL*);

GRID *creategrid(int /*rows*/, int /*cols*/, int /*gtype*/);
void freegrid(GRID *);


/* visit functions return a CELL pointer */
CELL *visitrc(GRID *, int /*rows*/, int /*cols*/);
CELL *visitid(GRID *, int /*cellid*/);
CELL *visitdir(GRID *, CELL */*cell*/, int/*direction*/, int/* connection status */);
CELL *visitrandom(GRID *);


/* bycell functions use one or two CELL pointers
 * byrc functions take GRID and one or two pairs of row,col
 * byid functions take GRID and one or two ids
 *
 * connect makes connections
 * isconnected tests connections
 * delconnect removes connections	(TODO)
 * edgestatus returns information about edges
 * wallstatus returns information about walls
 */
void connectbycell(CELL *, int /* cell1 -> cell2 direction */,
             CELL *, int /* cell2 -> cell1 direction */);

void connectbyrc(GRID *,
		int /*r1*/, int /*c1*/, int /* cell1 -> cell2 direction */,
             	int /*r2*/, int /*c2*/, int /* cell2 -> cell1 direction */);

void connectbyid(GRID *,
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

int wallstatusbycell(CELL *);
int wallstatusbyrc(GRID *, int /*row*/, int/*col*/);
int wallstatusbyid(GRID *, int /*id*/);

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
int iteraterow(GRID *, int, int(*)(GRID *,CELL*,void *), void *);
int iteratecol(GRID *, int, int(*)(GRID *,CELL*,void *), void *);
int iterategrid(GRID *, int(*)(GRID *,CELL*,void *), void *);

/* find the opposite of a direction */
int opposite( int /*dir*/);

/* turn a direction into a name */
const char *dirtoname(int /*dir*/);

/* naive ascii art version of a grid */
char *ascii_grid(GRID *, int /* use_name */);
#endif
