Mazing Library Reference
========================

Includes
--------

1. `grid.h` This is a standalone unit, implementing a rectangluar grid.
    I am undecided if a future extension to non-rectangles will use a
    different grid.h (precluding generating rectange and non rectangle
    mazes in the same program) or be included here (possibly breaking some
    of the existing code).
2. `distance.h` This requires grid definitions. It implements a robust
    solver (loop tolerant) and a structure for holding paths between
    points to represent a solution to a maze.
3. `mazes.h` This requires grid and distance definitions. It implements
    ways to make mazes on grid, some of which use the distance.h path
    structure.
4. `mazeimg.h` This requires grid and distance definitions (and libpng's
    `<png.h>`). It is presumed that any way to make images from mazes
    will need to sometimes print solved versions, hence the distance.h
    requirement.


Functions
---------

### Defined in `grid.h`:
* `GRID* creategrid(int rows, int cols, int type)`
  Creates a grid of `rows` x `cols`, initializing the grid and all
  cells to `type`. Returns NULL on failure.
* `void freegrid(GRID* grid)`
  Frees a grid, including name, user data, cell data, and cells.
  (If the grid or cell user data contains pointers to malloc()ed memory
  then the user should free that and set those pointers to NULL first.)
* `void initcell(CELL *cell, int type, int row, int col, int id)`
  Cell initializer, usually called from `initgrid()`
* `void freecell(CELL *cell)`
  Frees a cell, usually called from `freegrid()`
* `int copycell(CELL *src, CELL *dest, CELLCOPYCONFIG *conf)`
  Copies a cell using the config, intended for use by `copygrid()` and
  `pasteintogrid()` but may be useful elsewhere.
* `GRID *copygrid(GRID *grid, int includeuserdata)`
  Creates a new grid that is a copy of the original, optionally copying
  the user data pointers (but as those are opaque, the actual data
  cannot be copied).
* `int rotategrid(GRID *grid, int rotation)`
  Rotates (or transposes) a grid. This rewrites cell ids in the `dirs`
  connection array and moves name / cell type / and cell data to the
  rotated location, and changes the `rows` and `cols` of grid itself
  if appropriate. Returns 0 on success, returns NC on failure. On
  failure the original grid is unmodified. See the ROTATE_* and FLIP_
  defines for possible rotations.
* `int pasteintogrid(GRID *src, GRID *dest, int top, int left, int includeuserdata)`
  Pastes a grid into another grid. The source cannot extend past the
  borders of the destination, and will be positioned at the top,left
  cell in destination. If includeuserdata is set, those pointers
  will be copied (but the data itself is opaque).
* `CELL* visitid(GRID *grid, int id)`
  Returns a pointer to the cell with a given id or NULL. Fastest of the
  `visit` family of functions.
* `CELL *visitrc(GRID *grid, int row, int col)`
  Returns a pointer to the cell at row,col or NULL.
* `CELL* visitdir(GRID *grid, CELL *cell, int dir, int connection_status)`
  Returns a pointer to the cell in the given direction from a specified
  cell, subject to connection_status constraints. Or returns NULL.
  Status of NC: only succeeds if cells are not connected; ANY doesn't
  connection at all; SYMMETRICAL requires a connection in the given
  direction and a connection back; THIS requires only a connection in
  the given direction.
* `CELL *visitrandom(GRID *grid)`
  Visit a cell randomly, useful for some maze generators.
* `namegrid(GRID *grid, char*name)`
  Allocates memory and copies name to the name pointer of the grid.
* `int iterategrid(GRID *grid, IFUNC_P ifunc, void *param)`
  For every cell in the grid, calls `ifunc()` with three parameters:
  a pointer to the grid, a pointer to the specific cell, and a pointer
  to the param structure (for user use). `ifunc()` should return an
  integer, and iterategrid returns the sum of those.
* `int iteraterow(GRID *grid, int row, IFUNC_P ifunc, void *param)`
  Just like `iterategrid()`, but only on a single row.
* `int iteratecol(GRID *grid, int col, IFUNC_P ifunc, void *param)`
  Just like `iterategrid()`, but only on a single column.
* `int opposite(int dir)`
  For any direction `dir` return the opposite direction. Returns NC
  for unknown directions.
* `const char *dirtoname(int dir)`
  Returns an English name for a direction, eg for printf() during
  debugging. Returns "not a direction" for unknown directions.
* `char *ascii_grid(GRID *grid, int style)`
  Allocates memory for an ASCII art version of the grid, and returns
  the ASCII drawing. The `style` can be `PLAIN_ASCII` for an ordinary
  drawing or `USE_NAMES` to put the first three characters of the cell
  name in each ASCII cell. The user should `free()` the return value.

There are a number of function families in `grid.h` that do the same
thing but with different ways to refer to cells.
* `...bycell()` functions use CELL pointers
* `...byrc()` functions use row,col co-ordinates
* `...byid()` functions use cell ids

The families are:
* `connect...()` to create connections
* `disconnect...()` to remove connections
* `isconnected...()` to test connections
* `natdirection...()` to return the direction from one cell to another
* `edgestatus...()` returns bitflags about the edges around a cell
* `wallstatus...()` returns bitflags about the walls around a cell
* `ncount...()` returns a count of neighboring cells
* `name...()` puts a name on a cell

* `void connectbycell(CELL *cell1, int c1c2d, CELL * cell2, int c2c1d)`
* `void connectbyrc(GRID *grid, int row1, int col1, int c1c2d, int row2, int col2, int c2c1d)`
* `void connectbyid(GRID *grid, int id1, int c1c2d, int id2, int c2c1d)`
  Create a connection from the first to second cell in direction `c1c2d`
  and a return connection in `c2c1d`. Either direction can by `NC` not
  connect, and `c2c1d` can be SYMMETRICAL for an automatic normal
  return direction.

* `void disconnectbycell(CELL *cell1, int c1c2d, CELL * cell2, int c2c1d)`
* `void disconnectbyrc(GRID *grid, int row1, int col1, int c1c2d, int row2, int col2, int c2c1d)`
* `void disconnectbyid(GRID *grid, int id1, int c1c2d, int id2, int c2c1d)`
  Very much like the `connect()` family, removing the connections given
  with NC for no change and `c2c1d` optionally SYMMETRICAL to be the 
  opposite of `c1c2d`.

* `int isconnectedbycell(CELL *cell1, CELL *cell2, int dir)`
* `int isconnectedbyrc(GRID *grid, int row1, int col1, int row2, int col2, int dir)`
* `int isconnectedbyid(GRID *grid, int id1, int id2, int dir)`
  For a pair of cells, return the direction by which the first connects
  to the second if there is a connection. The `dir` is either a single
  direction to test or `ANYDIR` for all directions. Returns `NC` if not
  connected. (**Note** zero is a valid direction, so be careful using
  the results in an `if` statement.)

* `int natdirectionbycell(CELL *cell1, CELL *cell2)`
* `int natdirectionbyrc(GRID *grid, int row1, int col1, int row2, int col2)`
* `int natdirectionbyid(GRID *grid, int id1, int id2)`
  Returns the expected natural direction from cell1 to cell2.  This is
  NORTH, SOUTH, EAST, WEST if adjacent, SYMMETRICAL if the cells are
  the same, or NC.

* `int edgestatusbycell(GRID *grid, CELL *cell)`
* `int edgestatusbyrc(GRID *grid, int row, int col)`
* `int edgestatusbyid(GRID *grid, int id)`
* `int wallstatusbycell(CELL *cell)`
* `int wallstatusbyrc(GRID *grid, int row, int col)`
* `int wallstatusbyid(GRID *grid, int id)`
  These return bit flags about edge and wall status of cell. Edges and
  walls use the same bits, so the results can be bitwise or'ed together
  for barrier status. But edge and wall error conditions differ. There
  are also NO_WALLS and NO_EDGES bits which differ. See the 
  *_WALL defines.

* `int ncountbycell(GRID *grid, CELL *cell, int concern, int type)`
* `int ncountbyrc(GRID *grid, int row, int col, int concern, int type)`
* `int ncountbyid(GRID *grid, int id, int concern, int type)`
  Counting neighbors. Returns negative on error or 0 to 4 depending
  on neighbors or "concern". With `concern` of NEIGHBORS it returns the
  number of neighbors in natural direction. With `concern` of EXITS, it
  only counts natural neighbors if there is a connection to the
  neighbor. With concern` of OF_TYPE, it only counts natural neighbors
  if the neighbor has the given cell type.

* `int namebycell(CELL *cell, char *name)`
* `int namebyrc(GRID *grid, int row, int col, char *name)`
* `int namebyid(GRID *grid, int id, char *name)`
  For a given cell, allocates memory and copies the provided name to
  the name field for the cell.


### Defined in `distance.h`:

* `DMAP *createdistancemap(GRID *grid, CELL *cell)`
  Creates and initializes a distance map appropriate for a particular
  grid, and marks the cell as the root of the map. All distances are
  initialized to `NV` (not visited). Returns NULL on error.
* `DMAP *findlongestpath(GRID *grid, int type)`
  Creates a distance map with a path between two of the farthest apart
  points. The type field is required for MASKED mazes to indicate any
  cell type that is actually part of the maze. Returns NULL on error.
* `void freedistancemap(DMAP *dmap)`
  Frees a distancemap, including the path.
* `int distanceto(DMAP *dmap, CELL *cell, int lazy)`
  On a freshly initialized distancemap (from `createdistancemap()`),
  will attempt to make a distance map to the target cell. Returns 0
  if successful at reaching target and DISTANCE_ERROR otherwise.
  Sets target_id only if successful. When operating in LAZYMAP mode
  stops as soon as the target is found. In NONLAZYMAP, distances to
  every reachable cell are calculated.
* `int findpath(DMAP *dmap)`
  If `distanceto()` found a distance to the target, `findpath()`
  will work out the path from root to target. Returns DISTANCE_ERROR
  on error and zero on success.
* `int iteratewalk(DMAP *dmap, IFUNC_P ifunc, void *param)`
  Works just like the `iterate...()` functions of grid.h, but walks
  along the results of `findpath()`. Returns the sum of the `ifunc()`
  return values.
* `int namepath(DMAP *dmap, char *fname, char *mname, char *lname)`
  Uses `iteratewalk()` to name cells along a path. The `fname` is
  used for the root, the `lname` for the target, and `mname` for
  other steps along the path. If any are NULL, a stringified version
  of the step count is used instead. Returns a negative value on
  error, zero on success.
* `void ascii_dmap(DMAP *dmap)`
  A debugging tool, prints a distance map to STDOUT, and used in
  testdistance.c


### Defined in `mazes.h`:

* `void defaultmasksetting(MASKSETTING *masksetting)`
  Initializes the values of a `masksetting` structure to the default
  values.
* `int btreewalker(GRID *grid, CELL *cell, void*unused)`
  A `iterategrid()` callback maze generator creating a simple binary
  tree maze. Has distinctive north and east edge unbroken row/column.
* `int sidewinderwalker(GRID *grid, CELL *cell, void* status)`
  A `iterategrid()` callback maze generator creating a simple maze
  with a distinctive unbroken north edge row. The `status` is a
  sw_tree_status structure with the runlength initially set to zero.
* `int serpentine(GRID *grid, CELL *cell, void*unused)`
  A `iterategrid()` callback maze generator creating a plain winding
  path through the grid.
* `int hollow(GRID *grid, CELL *cell, HOLLOWCONFIG *hollowmode)`
  A `iterategrid()` callback maze generator creating a hollows. See
  HOLLOWCONFIG structure documentation for configuration. Using
  NULL for hollowmode os the same as HMODE_ALL.
* `int aldbro(GRID *grid, MASKSETTING *masksetting)`
  Named for David Aldous and Andrei Broder, this method visits
  cells randomly until all tovisit cells have been reached. All
  cells to include should be set to the UNVISITED type initially,
  and will become VISITED after this runs. Has no way to detect
  if some cells are unreachable, and may never return on poor
  choices of grid masking. The MASKED value is never directly
  used. Returns negative on error, zero on success.
* `int wilson(GRID *grid, MASKSETTING *masksetting)`
  The David Bruce Wilson method is a series of random walks, each
  ending when finding a perviously visited cell. Loops created during
  the walks are removed before carving the path. Has similar VISITED,
  UNVISITED and MASKED concerns as `aldbro()` and similar inability
  to handle poor grid masks. Returns negative on error, zero on success.
* `int huntandkill(GRID *grid, MASKSETTING *masksetting)`
  Hunt-and-kill alternates hunting for unseen spaces and killing
  them with a random walk ending when finding a perviously visited
  cell. The chief difference from `wilson()` is the non-random
  starts to new walks. It has the same mask setting needs and
  limitations. Returns negative on error, zero on success.
* `int backtracker(GRID *grid, MASKSETTING *masksetting)`
  The recursive backtracker is an improved `huntandkill()` that
  searches backward along the current VISITED path for a new branch
  point instead of searching the grid for an UNVISITED spot. It
  has the same mask setting needs and limitations as `huntandkill()`
  and `wilson()`. Returns negative on error, zero on success.


### Defined in `mazeimg.h`:

There are two ways images are represented within this code.

1. There is a packed libpng compatible binary format for a whole maze.
   This is an array of pointers to row data. This means one contiguous
   block of memory only contains a single row. Within each row,
   everything is packed as tightly as possible for the colordepth and
   channels. This means a single byte can have as many as 8 pixel
   values.
2. There is an array of unsigned char "cellimage" format for single
   cells. This is a single contiguous block of memory for a rectangle.
   It uses 1 char per color per pixel for all bit depths up to 8, and 2
   chars per color per pixel for depth 16. This is meant to be an easy
   to use block of memory for a user to draw on, and then use the
   `placerectangle()` function (either directly or indirectly, as with
   `drawandplacecell()`).

* `MAZEBITMAP *createmazebitmap(DMAP *dmap)`
   Creates an empty maze bitmap structure from a distance map of a maze.
* `void freemazebitmap(MAZEBITMAP *mazebitmap)`
   Frees a maze bitmap (but not the associated distance map).
* `int initmazebitmap(MAZEBITMAP *mazebitmap, int height, int width, int colortype, int colordepth, int cellormaze)`
   For given parameters, set up informational values in MAZEBITMAP and
   allocate memory. Color type is one of COLOR_G, COLOR_GA, COLOR_RGB,
   or COLOR_RGBA and colordepth is one of 1, 2, 4, 8, or 16 (bits per
   color per pixel). All color types support 8 or 16 bits, COLOR_G
   supports all depths. The height and width can be for a single cell
   or the whole maze controlled by CELL_SIZE or MAZE_SIZE for the
   `cellormaze` parameter.
* `void fill_cell(png_byte *cellimage, int count, int channels, int twofer, int *colors)`
   Completely fill a `cellimage` with a single color The cellimage has
   `count` pixels, `channels`, has `twofer` set if two bytes per
   channel, and `colors` is an array of ints representing each color
   channel in order (eg, red, green, blue, and alpha).
* `void draw_a_line(png_byte *cellimage, int r1, int c1, int r2, int c2, int cwide, int channels, int twofer, int *colors)`
   Draw a horizontal or vertical line on a `cellimage`, from point
   r1,c1 to r2,c2. The cell width is in `cwide`; `channels`, `twofer`,
   and `colors` set as in `fill_cell()`.
* `void default_colorpicker(MAZEBITMAP *mazebitmap, CELL *cell, COLORDATA *cd)`
   Passed in a MAZEBITMAP, a CELL, and a COLORDATA structure, this picks a
   set of colors for the current cell. In RGB mode, if the distance map for
   the MAZEBITMAP is interesting, the background color of cells will vary
   with distance from the root cell.
* `int default_drawcell(MAZEBITMAP *mazebitmap, png_byte *cellimage, CELL *cell)`
   Passed in a MAZEBITMAP, a cellimage block of memory, and a CELL,
   this will draw a the selected cell into the cellimage. If the 
   MAZEBITMAP does not have a colorpicker defined (`colorfunc`), the
   `default_colorpicker()` will be used.
* `void placerectangle(png_byte *cellimage, int img_w, int img_h, png_bytep *allrows, int top, int left, int depth, int channels)`
   Passed in a `cellimage` (or other rectangle) that has been draw on,
   of size `img_w` x `imh_h`, the `rowsp` pointer from MAZEBITMAP or
   similar libpng `allrows` pointer, the `top`-`left` corner position of
   the placement for the cell image, and the depth and channel count,
   this will do the the fiddly work of pasting the cellimage into the
   libpng image structure.
* `int drawandplacecell(GRID *grid, CELL *cell, MAZEBITMAP *mazebitmap)`
   This is a `iterategrid()` callback which will call the MAZEBITMAP
   `cellfunc()` to draw each cell and then `placerectangle()` to put it
   into the full image.
* `int drawmaze(MAZEBITMAP *mazebitmap)`
   This will set a default `cellfunc()` drawing function if one is not
   yet provided in MAZEBITMAP, then `iterategrid()` that function.
* `int writepnm(MAZEBITMAP *mazebitmap, char *filename)`
   This will write the image part of a MAZEBITMAP to the given
   `filename`. The format will be PBM for 1 bit grayscale, PGM for 2 to
   16 bit grayscale, PPM for 8 or 16 bit RGB, and PAM for all types
   with alpha channels.
* `int writepng(MAZEBITMAP *mazebitmap, char *filename)`
   This will PNG encode the image part of a MAZEBITMAP to the given
   `filename`.


Data types
----------

### Defined in `grid.h`:
* `CELL`
   Treat as read-only:
   * `int id;`
      cell unique identifier (and index into grid array)
   * `int row,col;`
      the notional location of the cell
   * `int dir[DIRECTIONS];`
      links to other cells in each of the DIRECTIONS
   Read-write:
   * `int ctype;`
     Initialized to be the same as grid type, and used by some maze
     generators. VISITED, UNVISITED, and MASKED are values generators
     may care about, where *visited* status refers to the state of
     the generator, not the user. Post maze generation, is not used
     by other mazing functions.
   * `char *name;`
     There is a helper function to set this, and optionally printed by the
     ASCII art maze printer.
   * `void *data;`
     This is purely for user use.

* `GRID`
   Treat as read-only:
   * `int rows, cols;`
     Dimensions of the maze grid.
   * `int planes;`
     For future use.
   * `int max;`
     Size of a single plane (rows x cols)
   * `CELL *cells;`
     Array of cells, indexed by id.
   Read-write:
   * `int gtype;`
     Initialized during grid creation, but never subsequently used. May
     be read by future grid-to-image functions. (Think non-rectangular.)
   * `char *name;`
     Can be set with a helper function, and optionally printed by the
     ASCII art maze printer.
   * `void *data;`
     This is purely for user use.

* `CELLCOPYCONFIG`
  This is a configuration structure entirely for user specified values.
  * `int origwidth;`
     This is the rows of the grid being copied from.
  * `int newwidth;`
     This is the rows of the grid being copied to.
  * `int rowoffset;`
     This is how much the row changes for a co-ordinate.
  * `int coloffset;`
     This is how much the col changes for a co-ordinate.
  * `int includeuserdata;`
     This is a flag for copying the user data pointers.

### Defined in `distance.h`:
* `TRAIL`
  Used for storing a path (or walk) through a maze. Can be used for
  user's own walk structures, but the path in a distance map version
  is read-only.
  * `int cell_id;`
  * `struct trail_t *next;`
  * `struct trail_t *prev;`

* `DMAP`
  A distance map, used to show the distance between one point and many
  other points, possibly even the full maze. This is entirely read-only.
  * `GRID *grid;`
    A pointer to the GRID this map is for.
  * `int root_id;`
    The zero point: where all distances are measured from.
  * `int rrow, rcol;`
    Co-ordinates of the zero point.
  * `int target_id;`
    The point of interest: will be negative if the distance hasn't been
    successfully calculated.
  * `int farthest_id;`
    (One of) the farthest point from the zero point. It's possible for
    multiple points to be equidistant, but this will be one of those.
    Only set with the NONLAZYMAP option.
  * `int farthest;`
    How far away that farthest point is. Only set with the NONLAZYMAP
    option.
  * `int msize;`
    Number of points to malloc for a map.
  * `int *map;`
    Distances, indexed by cell id.
  * `int *frontier;`
    Cell ids to check during the next round.
  * `TRAIL *path;`
    Linked list of a path from root to target. next/prev set to NULL 
    at the ends.


### Defined in `mazes.h`:
* `MASKSETTING`
  Used during maze construction to redefine defaults for VISITED,
  UNVISITED, and MASKED, and also to provide a count of cells to
  be included in the maze, if different from total number of cells.
  Completely user settable, with a function to initialize to defaults.
  * `int type_unvisited;
  * `int type_visited;
  * `int type_masked;
  These are cell types for the different statuses. VISITED/UNVISITED
  refer to the maze generator's status for a cell, and will be set
  during generation. MASKED are cells to not use. Most generators
  actually compare either VISITED/UNVISITED or MASKED but not both.
  But it's usually critical that all three values differ.
  * `int to_visit;`
  Several of the generators rely on knowing a count of cells left to
  include in the maze, if cells have been MASKED out this needs to
  be less than the total number of cells.

* `HOLLOWCONFIG`
  The `hollow()` "maze" generator just "knocks down" all walls. It
  has a limited amount of configurability. You can knock down the
  walls only for cells of a certain type (or not a certain type)
  and control if that knocking down includes neighbors not of that
  specified type. Or you can knock down everything.
  * `int mode;`
    Use HMODE_ALL, HMODE_SAME_AS, HMODE_SAME_AS_STRICT, HMODE_DIFFERENT
    or HMODE_DIFFERENT_STRICT. The strict versions check cell types at
    both ends of the connection.
  * `int ctype;`
    Used if not HMODE_ALL


### Defined in `mazeimg.h`:
* `MAZEBITMAP`
  Treat as read-only:
  * `int rows, cols;`
    Copied from the GRID, set during initialization.
  * `int cell_w, cell_h;`
  * `int img_w, img_h;`
    Set during initialization, one pair from parameters and the other
    calculated based on the CELL_SIZE / MAZE_SIZE setting.
  * `int colortype;`
    Set during initialization: COLOR_G, COLOR_GA, COLOR_RGB or
    COLOR_RGBA.
  * `int colordepth;`
    Set during initialization: 1, 2, 4, 8, or 16. This is bits per
    color channel per pixel.
  * `int channels;`
    Set during initialization: 1, 2, 3, or 4. Each of red, green, blue,
    gray, and alpha is a channel. 1 or 2 is a grayscale image, without
    or with a alpha. 3 or 4 is RGB without or with alpha.
  * `int doubled;`
    Set during initialization if colordepth is 16.
  * `int rowsize;`
  * `int cellsize;`
    These are sizes suitable for use with `malloc()`
  * `DMAP *dmap;`
    This is created from a distance map, and a pointer to the one used
    is stored here; that in turn has a pointer to the GRID used. The
    default cell drawing method likes to colorize by distance map, and
    any method to draw a solution will need the `findpath()` results.
  * `png_bytep *rowsp;`
    A list of pointers to each row in the raw binary format favored
    by libpng for image data.
  Read-write:
  * `CELLFUNC_P cellfunc;`
    This is a function pointer to a call back to draw a single cell.
    The function should return an int, and take three pointers:
    a MAZEBITMAP pointer (points back to this structure, for distance
    map and grid access); a cellimage array (raw data to be written to
    with a picture of a cell), and a CELL pointer to the cell to be
    drawn. The `drawmaze()` function will return the sum of all of
    calls to this function, the default drawing function returns 1 on
    every successfully drawn cell. If this pointer is not set, then
    `drawmaze()` will set it.
  * `COLORFUNC_P colorfunc;`
    This is a function pointer to a call back to set colors for drawing
    a cell. It does not return any value, but alters a structure passed
    in. The function should take pointers to the MAZEBITMAP, a CELL, and
    a COLORDATA structure, to modify the user writable values there. If
    this pointer is not set, the default maze drawer will NOT set this,
    but will use a default color setter.
  * `void *udata;`
    This is anything the user wants, but the user must `free()` it.

* `COLORDATA`
  This is the structure used by the default color setter and the default
  cell drawing function. Only the low `depth` number of bits will be
  used from any color value. 0x1000 is black for a depth of under 8.
  Treat as read-only:
  * `int channels;`
    Color channel count: 1: gray; 2: gray alpha; 3: RGB; 4 RGBA. These
    are the orders to set color values: with `channels` set to 2,
    `wall[0]` should be the gray value and `wall[1]` the alpha. With
    channels` set to 3, `wall[0]`, `wall[1]`, `wall[2]` are the red,
    green, and blue respectively.
  * `int depth;`
    Number of bits for each color: 1, 2, 4, 8, or 16.
  To be filled in by a color setter:
  * `int wall[4];`
    Interior wall colors in expected order.
  * `int edge[4];`
    Outer edge colors in expected order.
  * `int fg[4];`
  * `int bg[4];`
    Foreground/background colors.
  * `int uc1[4];`
  * `int uc2[4];`
  * `int uc3[4];`
  * `int uc4[4];`
    User-use colors.


Defined Values
--------------

### Defined in `grid.h`:
* `DIRECTIONS`
   Number of defined directions, and 1 more than highest direction index.
* `FOURDIRECTIONS`
   One more than the highest n/s/w/e index (for use in loops).
* `FIRSTDIR`
   The lowest direction index (for use in loops).
* `NORTH`
* `SOUTH`
* `EAST`
* `WEST`
   The four cardinal directions.
* `UP`
* `DOWN`
   Future use directions.
* `NC`
   No connection (less than all direction values), used in creating
   connections and as a return value.
* `SYMMETRICAL`
   For requesting a symmetric connection between two cells. Less than
   all direction values.
* `ANYDIR`
   For a request that doesn't need any particular direction. Less than
   all direction values.
* `ANY`
   Same as `ANYDIR`, but also used to mean "any connection status".
* `EITHER`
   Not currently used.
* `THIS`
   For a request than needs the specified direction *only*. Less than
   all direction values.
* `EXITS`
   Only consider directions that have connections.
* `NEIGHBORS	-7
   Only consider directions that are adjacent.
* `OF_TYPE`
   Only consider neighbors that have the given cell type.
* `ROTATE_CW
   Rotate 90 degrees (conviently is 90)
* `CLOCKWISE         ROTATE_CW
   Rotate 90 degrees.
* `ROTATE_CCW`
   Rotate 270 degrees (conviently is -90)
* `ROTATE_CCW_ALT`
   Rotate 270 degrees (conviently is 270)
* `COUNTERCLOCKWISE
   Rotate 270 degrees.
* `ROTATE_180
   Rotate 1800 degrees (conviently is 180)
* `FLIP_LEFTRIGHT`
   Left to right transpose.
* `FLIP_TOPBOTTOM`
   Top to bottom transpose.
* `FLIP_TRANSPOSE`
   Diagonal transpose (same as doing both a CCW rotate and a LR transpose)
* `NORTH_EDGE` or `NORTH_WALL`
* `WEST_EDGE` or `WEST_WALL`
* `EAST_EDGE` or `EAST_WALL`
* `SOUTH_EDGE` or `SOUTH_WALL`
   Bit mask for a wall or edge (either boundary) in that direction.
* `NORTHWEST_CORNER`
* `NORTHEAST_CORNER`
* `SOUTHWEST_CORNER`
* `SOUTHEAST_CORNER`
   Bit mask for a wall or edge (either boundary) in both of those
   directions.
* `NO_WALLS`
   Bit mask for no walls on the current cell (different than no edges)
* `NO_EDGES`
   Bit mask for no edges on the current cell (different than no walls)
* `EDGE_ERROR` `WALL_ERROR`
   Bit mask for an error response.
* `PLAIN_ASCII`
   Simpler output option for `ascii_grid()`
* `USE_NAMES`
   Has `ascii_grid()` put the first three characters of the name in
   each cell.
* `BUFSIZ`
   Set only if not already defined, maximum size for a name field.
   <stdio.h> usually defines this to 1024 or larger.


### Defined in `distance.h`:
* `LAZYMAP`
   Used to create a minimal effort distance map during `distanceto()`
* `NONLAZYMAP`
   Used to create a thourough distance map during `distanceto()`. The
   Non-lazy method is required to find points farthest apart.
* `DISTANCE_NOPATH`
   A no-path found distance map return code.
* `DISTANCE_ERROR`
   A distance map return code for general errors.
* `NV`
   Aka `NOT_VISITED`, a value used in distance maps.
* `NOT_VISITED`
   Aka `NV`, a value used in distance maps.
* `FRONTIER`
   A value used in distance maps to mark the edges of explored space.


### Defined in `mazes.h`:
* `UNVISITED` and `VISITED`
   Cell types used to indicate which cells have yet to be incorporated
   into a maze or have been added already.
* `MASKED`
   Cell type to be ignored during maze making. Needs to be different
   than UNVISITED/VISITED.
* `NEEDDIR`
   Larger than all direction indexes, for use in picking a suitable
   direction.
* `HMODE_ALL`
   For `hollow()`, hollow out everything.
* `HMODE_SAME_AS`
* `HMODE_SAME_AS_STRICT`
   For `hollow()`, hollow out cells of a matching type. Strict checks
   current and connected to cells.
* `HMODE_DIFFERENT`
* `HMODE_DIFFERENT_STRICT`
   For `hollow()`, hollow out cells not of a matching type. Strict
   checks current and connected to cells.


### Defined in `mazeimg.h`:
* `CELL_SIZE`
   Size values are for a single cell. Complete maze values will be
   calculated.
* `MAZE_SIZE`
   Size values are for a complete cell. Single cell values will be
   calculated (rounding down).
* `COLOR_G`
   Gray values only. Can be any bit depth: 1, 2, 4, 8, or 16.
* `COLOR_GA`
   Gray value with an alpha channel. Can be bit depth: 8 or 16.
* `COLOR_RGB`
   Red, Green, and Blue values. Can be bit depth: 8 or 16.
* `COLOR_RGBA`
   Red, Green, and Blue values with an alpha channel. Can be bit
   depth: 8 or 16.


