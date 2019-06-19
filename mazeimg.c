/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* ways to make a maze grid into an image */
/* uses libpng.h */

#include <stdlib.h>
#include <png.h>

#include "grid.h"
#include "distance.h"
#include "mazes.h"		/* TESTING */

/* for intepreting height and width in initmazebitmap */
#define CELL_SIZE	5
#define MAZE_SIZE	6

/* used internally for draw_a_line() */
#define WALL_LINE	255
#define EDGE_LINE	127
#define ALPHA_OPAQUE	0xff


#define COLOR_G 	PNG_COLOR_TYPE_GRAY
#define COLOR_GA        PNG_COLOR_TYPE_GRAY_ALPHA
#define COLOR_RGB       PNG_COLOR_TYPE_RGB
#define COLOR_RGBA      PNG_COLOR_TYPE_RGB_ALPHA

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
   * 800 byte (10x10x4x2) cellimage.
   */
  int (*cellfunc)(struct mazebitmap_s *, void */*cellimage*/, CELL */*cell*/);

  void *udata; /* A pointer to a user data structure */

} MAZEBITMAP;

MAZEBITMAP *
createmazebitmap(DMAP *dm)
{
  MAZEBITMAP *mb;

  if(!dm) { return (MAZEBITMAP*)NULL; }

  mb = (MAZEBITMAP *)malloc( sizeof(MAZEBITMAP) );
  if(!mb) { return (MAZEBITMAP*)NULL; }

  /* we keep local copies of rows/cols in case the distance map
   * has been free()ed before freemazebitmap()
   */
  mb->rows = dm->grid->rows;
  mb->cols = dm->grid->cols;
  mb->dmap = dm;

  mb->cell_w = mb->cell_h =
  mb->img_w  = mb->img_h  =
  mb->colortype           =
  mb->colordepth          = 0;

  mb->rowsp    = (png_bytep*)NULL;
  mb->cellfunc = NULL;
  mb->udata    = NULL;

  return mb;
} /* createmazebitmap() */

/* Caller is responsible for freeing the distance map and user data */
void
freemazebitmap(MAZEBITMAP *mb) {
  int i;

  if(!mb) { return; }

  if(mb->rowsp) {
    for (i=0; i < mb->rows; i++) {
      free(mb->rowsp[i]);
    }
    free(mb->rowsp);
  }
} /* freemazebitmap() */

/* Takes a height and width, a color type, a color depth, and a
 * flag CELL_SIZE or MAZE_SIZE to interpret the height and width,
 * then allocates the memory for the image.
 * Returns a negative number on failure.
 * Returns 1 on "all good".
 * Returns 0 if bitmap options incompatible with PNG. (Still okay for PNM.)
 */
int
initmazebitmap(MAZEBITMAP *mb, int h, int w, int ct, int cd, int cellormaze)
{
  int  sampleround;

  if(!mb) { return -1; }

  if(cellormaze == CELL_SIZE) {
    mb->cell_h = h;
    mb->cell_w = w;
    mb->img_h = mb->rows * h;
    mb->img_w = mb->cols * w;
  } else if(cellormaze == MAZE_SIZE) {
    mb->img_h = h;
    mb->img_w = w;
    mb->cell_h = h / mb->rows;	/* this will round down */
    mb->cell_w = w / mb->cols;	/* this will round down */
  } else {
    return -2;
  }

  switch (ct) {
    case COLOR_G:
    case COLOR_GA:
    case COLOR_RGB:
    case COLOR_RGBA: mb->colortype = ct;
                     break;

    default:         return -3;
  } /* checking color type */
  switch (cd) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 16: mb->colordepth = cd;
             break;

    default: return -4;
  } /* checking color depth */

  switch (ct) {
    case COLOR_G:    mb->channels = 1; break;
    case COLOR_GA:   mb->channels = 2; break;
    case COLOR_RGB:  mb->channels = 3; break;
    case COLOR_RGBA: mb->channels = 4; break;
  } /* set sample size */

  mb->cellsize = mb->cell_h * mb->cell_w * mb->channels;
  /* is this 16 bits? */
  if( cd > 8 ) {
    mb->cellsize *= 2;
    mb->doubled = 1;
  } else {
    mb->doubled = 0;
  }

  /* do we need a final byte for less than multiple of 8 pixels? */
  if( cd < 8 ) {
    sampleround = 1;
  } else {
    sampleround = 0;
  }

  mb->rowsize = (mb->img_w * mb->channels * cd) / 8 + sampleround;

  mb->rowsp = (png_bytep *)malloc( mb->img_h * sizeof(png_bytep) );
  if(!mb->rowsp) {
    return -5;
  }

  for(int i = 0; i < mb->img_h; i ++) {
    mb->rowsp[i] = (png_byte *)calloc( (size_t)1, (size_t)mb->rowsize );
    
    if(!mb->rowsp[i]) {
      /* oof */
      while( i ) {
        i --;
	free(mb->rowsp[i]);
      }
      free(mb->rowsp);
      return -5;
    } /* if allocation failed */
  } /* for each row */

  if( (cd < 8) && (ct != COLOR_G) ) {
    return 0;
  }
  return 1;
} /* initmazebitmap() */


/* extra complicated since trying to accomodate all color/depth pairs */
static
void
draw_a_line(png_byte *image, int r1, int c1,
                         int r2, int c2,
            int cc, int channels, int twofer,
	    png_byte linetype)
{
   int i, j, k, alpha, id, offset;
   char *writer;
   
   if((channels == 2) || (channels == 4)) {
     alpha = 1;
   } else {
     alpha = 0;
   }
   for( i = r1; i <= r2; i ++ ) {
     for( j = c1; j <= c2; j ++ ) {
       id = i * cc + j;
       offset = id * channels * twofer;
       writer = &(image[offset]);

       for (k = 0; k < (channels - alpha) * twofer; k ++) {
         writer[k] = linetype;
       }

       if(alpha) {
         writer[k] = ALPHA_OPAQUE;
	 if(twofer > 1) {
           writer[k+1] = ALPHA_OPAQUE;
	 }
       }
     }
   }
} /* draw_a_line() */

/* if cellfunc is undefined when asked to draw a maze, this drawing
 * routine kicks in.
 */
int 
default_drawcell(MAZEBITMAP *mb, png_byte *image, CELL *c)
{
  int walls, edges, i1, i2, j1, j2;
  int siz = mb->channels;
  int twofer = mb->doubled + 1;
  int wid = mb->cell_w;
  int hei = mb->cell_h;

  walls = wallstatusbycell(c);
  if(walls & NORTH_WALL) {
    i1 = i2 = 0;
    j1 = 0;
    j2 = wid - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, WALL_LINE);
  }
  if(walls & SOUTH_WALL) {
    i1 = i2 = hei - 1;
    j1 = 0;
    j2 = wid - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, WALL_LINE);
  }
  if(walls & WEST_WALL) {
    j1 = j2 = 0;
    i1 = 0;
    i2 = hei - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, WALL_LINE);
  }
  if(walls & EAST_WALL) {
    j1 = j2 = wid - 1;
    i1 = 0;
    i2 = hei - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, WALL_LINE);
  }

  edges = edgestatusbycell(mb->dmap->grid, c);
  if(edges & NORTH_EDGE) {
    i1 = i2 = 0;
    j1 = 0;
    j2 = wid - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, EDGE_LINE);
  }
  if(edges & SOUTH_EDGE) {
    i1 = i2 = hei - 1;
    j1 = 0;
    j2 = wid - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, EDGE_LINE);
  }
  if(edges & WEST_EDGE) {
    j1 = j2 = 0;
    i1 = 0;
    i2 = hei - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, EDGE_LINE);
  }
  if(edges & EAST_EDGE) {
    j1 = j2 = wid - 1;
    i1 = 0;
    i2 = hei - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, EDGE_LINE);
  }

  return 1;
} /* default_drawcell */

/* can place a cell image (or any other rectangle) onto the full
 * size image. This is divorced from the overall maze bitmap structure
 * just so it can be used for putting any rectangle into the image,
 * perhaps to draw a player on a rendered maze, for example.
 *
 * image is the rectangle to place, of size img_w x img_h
 * allrows is (eg) rowsp from a MAZEBITMAP
 * top is the top row in allrows to start placing the image
 * left is the pixel number (not byte offset) in the row to start
 * depth and channels are used to workout how many bytes per
 * 	pixel are used in image and the full picture
 *
 * This gets complicated because although the cell image is a super easy
 * to use format, the PNG functions want something else. They expect the
 * 1, 2, and 4 depth versions to be tightly packed: 8 x 1bit per byte,
 * 4 x 2bit, and 2 x 4bit. The code is divided between the easy (whole
 * byte) and complicated (bit fiddly) parts.
 */
void
placerectangle(png_byte *image,    int img_w, int img_h, 
	       png_bytep *allrows, int top,   int left,
	       int depth, int channels)
{
  int p;		/* position in cell image */
  int fsize;		/* size of a pixel in full image (depth >= 8) */
  int csize;		/* size of a pixel in cell image */

  png_byte* row;	/* pointer to a row of pixel data */
  png_byte* pixel;	/* pointer to start of a pixel */

  int row_n, col_n;	/* row and column number. */
  int copy;		/* size of a block to copy */
  int i, k, o;

  if( depth > 7 ) {
    /* easy case */
    if( depth == 8 ) {
      fsize = csize = channels;
    } else {
      fsize = csize = 2 * channels;
    }
    
    p = 0;
    copy = img_w * csize;
    for (i = 0; i < img_h; i ++) {
      row_n = i + top;
      row = allrows[row_n];
      col_n = left * fsize;

      pixel = &(row[col_n]);

      for(k = 0; k < copy; k ++) {
        pixel[k] = image[p];
	p++;
      }

    }
  } else {
    /* complicated case, just gray (that's simple) but multiple
     * pixels per byte (not so simple), with a write possibly not
     * operating on full bytes (urg).
     */

    int img_col;	 /* column of the image, not the bit placement */
    unsigned char accum; /* accumulates bits */
    unsigned char frag;  /* some bits to put in accum */
    int used;		 /* how many bits used in accum */
    int move;		 /* how far to shift bits */
    int adj;             /* how to find byte offset */
    unsigned char mask;  /* bitmask */

    switch (depth) {
       case 4:   move = 4;  mask = 0xf;  adj = 2;  break;
       case 2:   move = 2;  mask = 0x3;  adj = 4;  break;
       case 1:   move = 1;  mask = 0x1;  adj = 8;  break;
    }
    p = 0;
    for (i = 0; i < img_h; i ++) {
      row_n = i + top;
      row = allrows[row_n];
      img_col = left;
      col_n = left / adj;
      used  = left % adj;
      pixel = &(row[col_n]);
      o = 0;
      if(used) {
        accum = pixel[o];
	/* convert that remainder into a pixel count */
	used = used * 8 / adj;
      } else {
        accum = 0;
      }

      for(k = 0; k < img_w; k++) {
        frag = image[p] & mask;

	switch(used) {
	  /* these only happen for single bit depths */
	  case 1:  frag <<= 6;                     /* 0000 0001 -> 0100 0000 */
	           accum |= frag;
	           used += move;
		   break;
		  
	  case 3:  frag <<= 4;                     /* 0000 0001 -> 0001 0000 */
	           accum |= frag;
	           used += move;
		   break;
		 
	  case 5:  frag <<= 2;                     /* 0000 0001 -> 0000 0100 */
	           accum |= frag;
	           used += move;
		   break;
		
	  case 7:  accum |= frag;
	           used += move;
		   break;

	  /* these happen for single and two bit depths */
	  case 2:  frag <<= 4;                     /* 0000 0001 -> 0001 0000 */
	           if( move == 1 ) { frag <<= 1; } /* 0001 0000 -> 0010 0000 */
	           accum |= frag;
	           used += move;
		   break;
		   
	  case 6:  if( move == 1 ) { frag <<= 1; } /* 0000 0001 -> 0000 0010 */
	           accum |= frag;
	           used += move;
		   break;

	  /* these happen for all bit depths */
	  case 0:  if( move == 1 ) { frag <<= 7; } /* 0000 0001 -> 0000 0010 */
	           else
	           if( move == 2 ) { frag <<= 6; } /* 0000 0001 -> 0100 0000 */
		   else
		   /* move == 4 */ { frag <<= 4; } /* 0000 0001 -> 0001 0000 */
	           accum |= frag;
	           used += move;
		   break;

	  case 4:  if( move == 1 ) { frag <<= 3; } /* 0000 0001 -> 0000 1000 */
	           else
	           if( move == 2 ) { frag <<= 2; } /* 0000 0001 -> 0000 0100 */
		   /* else do nothing for move == 4 */
	           accum |= frag;
	           used += move;
		   break;
	} /* bits used in accum */

	if (used == 8) {
	  pixel[o] = accum;
	  o ++;
	  used = accum = 0;
	}

	p++;
      } /* for k (over cell image cols) */

      /* do we need to write back a partially filled byte? */
      if(used) {
	pixel[o] = accum;
	used = accum = 0;
      }

    } /* for i (over cell image rows) */

  } /* depth less than 8 bytes per channel */

} /* placerectangle() */

/* Calls the current cellfunc() draw function to draw a cell,
 * then places that on the full bitmap.
 * If cellfunc() returns a negative number, this will abort before
 * placing that cell, otherwise returns -100 on memory error, or
 * the return value of cellfunc().
 * Note that iterategrid() does not abort on negative return values
 * and it returns the sum of all return values of the iterated
 * function (of which drawandplacecell() is one such function).
 */
int
drawandplacecell(GRID *g,CELL *cell,MAZEBITMAP *mb)
{
  int rc = 0;
  png_byte *image;

  /* calloc'ing the initial cell image results in it starting fully black 
   * (and the alpha channel, if there is one, to fully transparent)
   */ 
  image = calloc( (size_t)1, (size_t)mb->cellsize );
  if(!image) { return -100; }

  /* draw */
  rc = (mb->cellfunc)(mb, image, cell);
  if(rc < 0) { return rc; }

  /* place */
  placerectangle(image, mb->cell_w, mb->cell_h, mb->rowsp,
                       cell->row * mb->cell_h, cell->col * mb->cell_w,
                       mb->colordepth, mb->channels);


  free(image);
  return rc;
} /* drawandplacecell() */


int
drawmaze(MAZEBITMAP *mb)
{
  int rc;
  if(!mb)    { return -1; }

  if(!mb->cellfunc) {
    mb->cellfunc = /* type casting function pointers is verbose */
    		   (int(*)(MAZEBITMAP *, void *, CELL *)) default_drawcell;
  }
  rc = iterategrid( mb->dmap->grid, 
		    /* type casting function pointers is verbose */
  		    (int (*)(GRID *,CELL *,void *)) drawandplacecell,
		    mb);
  return rc;
} /* drawmaze */


/* only writes ascii PGM for now while testing drawing functions */
int
writepnm(MAZEBITMAP *mb, char *filename)
{
  if(!mb) { return -1; }

  FILE *fp = fopen(filename, "w");
  if(!fp) {
    fprintf(stderr, "failed to open %s", filename);
    return -1;
  }

  fprintf(fp, "P2\n%d %d\n%d\n", mb->img_w, mb->img_h, 255 /*maxval*/);

  for(int r = 0; r < mb->img_w; r++) {
    png_byte *row = mb->rowsp[r];
    for(int c = 0; c < mb->img_h; c++ ) {
      fprintf(fp, "%3d ", row[c]);
    }
    fprintf(fp, "\n");
  }

  fclose(fp);
  return 0;

} /* writepnm() */

/* Write a PNG version of a maze bitmap.
 * This is (currently) the only function that actually uses libpng
 * functions (as opposed to mere #defines).
 * returns a 0 on success, or a negative value on failure.
 */
int
writepng(MAZEBITMAP *mb, char *filename)
{
  png_structp png_ptr;
  png_infop info_ptr;

  if(!mb) { return -1; }

  FILE *fp = fopen(filename, "w");
  if(!fp) {
    fprintf(stderr, "failed to open %s", filename);
    return -1;
  }

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png_ptr) {
    fprintf(stderr, "create png write struct failed\n");
    return -2;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if(!png_ptr) {
    fprintf(stderr, "create png info struct failed\n");
    return -2;
  }

  /* libpng likes to return from errors with longjmp(), which returns to
   * a previously specified setjmp(). Setting a jump point always succeeds,
   * setjmp() returns an error after longjmp() is called.
   */
  if (setjmp(png_jmpbuf(png_ptr))) {
    fprintf(stderr, "init png io failed\n");
    fclose(fp);
    return -3;
  }
  png_init_io(png_ptr, fp);

  if (setjmp(png_jmpbuf(png_ptr))) {
    fprintf(stderr, "write png header failed\n");
    fclose(fp);
    return -4;
  }
  png_set_IHDR(png_ptr, info_ptr, mb->img_w, mb->img_h,
               mb->colordepth, mb->colortype, 
	       PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
	       PNG_FILTER_TYPE_BASE);
  png_write_info(png_ptr, info_ptr);

  if (setjmp(png_jmpbuf(png_ptr))) {
    fprintf(stderr, "write png body failed\n");
    fclose(fp);
    return -5;
  }
  png_write_image(png_ptr, mb->rowsp);

  if (setjmp(png_jmpbuf(png_ptr))) {
    fprintf(stderr, "finish write png failed\n");
    fclose(fp);
    return -6;
  }
  png_write_end(png_ptr, NULL);

  fclose(fp);
  return 0;
} /* writepng() */


int
main()
{
  GRID *g;
  CELL *c;
  DMAP *dm;
  MAZEBITMAP *mb;
  int rc, errorgroup = 1;

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

  rc = initmazebitmap(mb, 15, 15, COLOR_G, 1, CELL_SIZE);
  if( rc < 0 ) {
    printf("init mazebitmap failed %d\n", rc);
    return errorgroup;
  } else if (rc == 0){
    printf("init mazebitmap returned 0, good for only PNM\n");
  } else if (rc == 1){
    printf("init mazebitmap returned 1, good for all images\n");
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

// need to finish writepnm() for cases other than 8-bit gray scale
//  rc = writepnm(mb, "tmp.pgm");
//  if(!rc) {
//    printf("Wrote tmp.pgm\n");
//  }
  rc = writepng(mb, "tmp.png");
  if(!rc) {
    printf("Wrote tmp.png\n");
  }

  freemazebitmap(mb);
  freedistancemap(dm);
  freegrid(g);
  return 0;
}
