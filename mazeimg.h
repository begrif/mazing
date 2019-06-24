/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* ways to make a maze grid into an image */
/* uses libpng.h */

#ifndef _MAZEIMG_H
#define _MAZEIMG_H

#include <png.h>

#include "grid.h"
#include "distance.h"

/* for intepreting height and width in initmazebitmap */
#define CELL_SIZE	5
#define MAZE_SIZE	6

/* used internally for default_colorpicker() */
/* these are doubled up for 16 bit, eg, GRAY_BG becomes 0x4040 */
#define ALPHA_OPAQUE     0xff

#define GRAY_WALL_LINE	 0x7f	/* white, unless >= 8 bits */
#define GRAY_EDGE_LINE	 0x00	/* black all bit depths */
#define GRAY_FG          0xff	/* white all bit depths */
#define GRAY_BG          0x40	/* black unless >= 8 bits */

#define R_WALL_LINE      0x41    /* 0x41207a, a purple */
#define G_WALL_LINE      0x20    /* 0x41207a, a purple */
#define B_WALL_LINE      0x7a    /* 0x41207a, a purple */

#define R_EDGE_LINE      0x40    /* 0x406079, a slate blue */
#define G_EDGE_LINE      0x60    /* 0x406079, a slate blue */
#define B_EDGE_LINE      0x79    /* 0x406079, a slate blue */

#define R_FG             0xdf    /* 0xdffff0, a light blue */
#define G_FG             0xff    /* 0xdffff0, a light blue */
#define B_FG             0xf0    /* 0xdffff0, a light blue */

#define R_BG             0xff    /* 0xff8f00, an orange */
#define G_BG             0x8f    /* 0xff8f00, an orange */
#define B_BG             0x00    /* 0xff8f00, an orange */


#define COLOR_G 	PNG_COLOR_TYPE_GRAY
#define COLOR_GA        PNG_COLOR_TYPE_GRAY_ALPHA
#define COLOR_RGB       PNG_COLOR_TYPE_RGB
#define COLOR_RGBA      PNG_COLOR_TYPE_RGB_ALPHA

/* Color structure used by default color picker.
 * Set channels and depth before calling the color picker,
 * get back all the colors. Not all colors might be used
 * in a particular drawing style.
 */
typedef struct colordata_s {
  int channels;		/* 1: gray; 2: gray alpha; 3: RGB; 4 RGBA */
  int depth;		/* 1 to 16 bits */
  int wall[4];		/* wall color in expected order */
  int edge[4];		/* edge color in expected order */
  int fg[4];		/* foreground color in ...*/
  int bg[4];		/* background color in ...*/

  int uc1[4];		/* user colors for user uses */
  int uc2[4];		/* user colors for user uses */
  int uc3[4];		/* user colors for user uses */
  int uc4[4];		/* user colors for user uses */
} COLORDATA;

typedef struct mazebitmap_s {
  int rows, cols;
  int cell_w, cell_h;
  int img_w, img_h;

  int colortype; /* COLOR_G, COLOR_GA, COLOR_RGB, COLOR_RGBA
                  * The channels are listed in order expected,
		  * eg, alpha channel always comes last
		  */

  int colordepth; /* 1, 2, 4, 8, 16 */
  		/* But note PNG has type/depth limitations:
		 * Gray: any depth
		 * Gray with Alpha: only 8 or 16
		 * RGB: only 8 or 16
		 * RGB with Alpha: only 8 or 16
		 */

  int channels;  /* each of Red, Green, Blue, Gray and Alpha is a channel */
  int doubled;     /* set if colordepth > 8 */

  int rowsize;	/* cols * (sizeof color type) */
  int cellsize; /* size of the raw cell for drawcell functions.
                 * cell_w * cell_h * channels * bytesperchannel
                 * COLOR_G has one channel, RGBA has four channels
		 * colordepth of 1, 2, 4, or 8 is one byte per channel,
		 * colordepth of 16 has two bytes per channel
		 * Channels are in order suggested by name, eg RGB is
		 * red, green, blue.
		 */
  DMAP *dmap;
  png_bytep *rowsp;	/* A list of pointers to each row */
  			/* Each row is basically the raw data */

  /* This is a call back to draw a single cell.
   * cellimage is a pre-allocated and zeroed block of memory with one or
   * two bytes per pixel per channel to be filled in as a raw cellimage.
   * So a 10 x 10 COLOR_G colordepth 1 image gets a 100 byte (10x10x1x1)
   * cellimage, while a 10 x 10 COLOR_RGBA colordepth 16 gets a 
   * 800 byte (10x10x4x2) cellimage. (This size is in cellsize, above.)
   *
   * drawandplacecell() (and by extension, drawmaze()) use this to fill
   * in the very raw cellimage array with a single rectangular cell and
   * put that on the full image. placerectrangle() is used to place the
   * cellimage. If the user doesn't supply this function, drawmaze() will
   * put a default in.
   */
  int (*cellfunc)(struct mazebitmap_s *, void */*cellimage*/, CELL */*cell*/);

  /* user creates a COLORDATA structure, sets the desired depth and channels
   * then this function will be called once per cell to get colors for
   * that cell. (At least that's how used by default drawing routine.)
   * If the user doesn't supply this function, a default will be used (but
   * this will not be set.)
   */
  void (*colorfunc)(struct mazebitmap_s *, CELL */*cell*/, COLORDATA *);

  /* A pointer to a user data structure (user responsible for free()ing) */
  void *udata;

} MAZEBITMAP;



/* Creates MAZEBITMAP structure and has a cursory initialization from
 * settings in a distance map.
 * returns NULL on error
 */
MAZEBITMAP *createmazebitmap(DMAP *);

/* Frees the contents of a MAZEBITMAP (excluding distance map and
 * user data)
 */
void freemazebitmap(MAZEBITMAP *);

/* Takes a height and width, a color type, a color depth, and a
 * flag CELL_SIZE or MAZE_SIZE to interpret the height and width,
 * then allocates the memory for the image.
 * Returns a negative number on failure.
 * Returns 1 on "all good".
 * Returns 0 if bitmap options incompatible with PNG. (Still okay for PNM.)
 */
int initmazebitmap(MAZEBITMAP *, int /* height */, int /* width */,
	int /* colortype */, int /* colordepth */, int /* cell-or-maze*/ );


/* Completely fill a cell with a color.
 * Count is the number of pixels in a cell, channels and twofer are used
 * to interpret the colors array, which is the same format as any of the
 * arrays in a COLORDATA struct.
 * channels is from MAZEDATA channels
 * twofer == 1 for 1 to 8 bit depth, and 2 for 16 bits.
 * Used by default_drawcell()
 */
void fill_cell(png_byte */*cellimage*/, int /* count */,
		int /* channels */, int /* twofer */, int */* colorarray */);


/* Draw a horizontal or vertical line in a cell a cell,
 * for r1 <= r2 and c1 <= c2. cwide is cell width, channels / twofer
 * used to understand the colors being used. The colors array is one
 * of the ones from the COLORDATA struct (or same format).
 * channels is from MAZEDATA channels
 * twofer == 1 for 1 to 8 bit depth, and 2 for 16 bits.
 * Used by default_drawcell()
 */
void draw_a_line(png_byte */* cellimage */,
                 int /*r1*/, int /*c1*/, int /*r2*/, int /*c2*/,
                 int /* cwide */, int /* channels */, int /* twofer */,
	         int */* colorarray */);

/* If colorfunc is undefined when default_drawcell is used to draw a maze,
 * this colorpicker routine kicks in. It uses the channels and depth values
 * in the COLORDATA struct to set the walls, edges, fg, and bg arrays.
 * In color images with non-trival distance maps, bg color is scaled to
 * distance.
 */
void default_colorpicker(MAZEBITMAP *, CELL *, COLORDATA *);

/* can place a cell image (or any other rectangle) onto the full
 * size image. This is divorced from the overall maze bitmap structure
 * just so it can be used for putting any rectangle into the image,
 * perhaps to draw a player on a rendered maze, for example.
 *
 * cellimage is the rectangle to place
 *      rectangle of size img_w x img_h
 *      structured as a one dimensional array
 * allrows is (eg) rowsp from a MAZEBITMAP
 * top is the top row in allrows to start placing the image
 * left is the pixel number (not byte offset) in the row to start
 * depth and channels are used to workout how many bytes per
 * 	pixel are used in image and the full picture
 */
void placerectangle(png_byte */* cellimage */, int /* img_w */, int /* img_h */,
	       png_bytep */* allrows */, int /* top */, int /* left */,
	       int /* depth */, int /* channels */);

/* Calls the current cellfunc() draw function to draw a cell,
 * then places that on the full bitmap, designed for use with iterategrid().
 *
 * If cellfunc() returns a negative number, this will abort before
 * placing that cell, otherwise returns -100 on memory error, or
 * the return value of cellfunc().
 *
 * Note that iterategrid() does not abort on negative return values
 * and it returns the sum of all return values of the iterated
 * function (of which drawandplacecell() is one such function).
 */
int drawandplacecell(GRID *, CELL *, MAZEBITMAP *);

/* Draws a maze into the maze bitmap. If the cellfunc is not defined,
 * supplies a default. Then iterates the drawandplacecell() function.
 */
int drawmaze(MAZEBITMAP *);

/* PNM is a family of easy to read / write file formats that are terrible
 * long term use, but good for easy file interchange between programs.
 * The "N" stans for "aNy".
 *
 * PBM is for 1 bit images ("bitmaps" in PNM nomenclature)
 * PGM is for grayscale ("graymaps" with no alpha channel)
 * PPM is for RGB ("pixmaps" with no alpha channel)
 * PAM is for everything else ("anymaps")
 *
 * PAM is used here only for images with alpha channels, it's much newer
 * and not supported by as many programs.  My installed version of Gimp,
 * for example doesn't recognize it, but my installed Imagemagick's convert
 * will read them.
 *
 * The user is expected to provide a suitable suffix, if one is desired.
 * A filename of '-' will write to STDOUT instead of a file, useful for
 * pipelines.
 * returns a 0 on success, or a negative value on failure.
 */
int writepnm(MAZEBITMAP *mb, char *filename);

/* Write a PNG version of a maze bitmap.
 * This is (currently) the only function that actually uses libpng
 * functions (as opposed to libpng #defines and data types).
 * returns a 0 on success, or a negative value on failure.
 */
int writepng(MAZEBITMAP *mb, char *filename);

#endif
