/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */
/* ways to make a maze grid into an image */
/* uses libpng.h */

#include <stdlib.h>

#include "mazeimg.h"


/* Creates MAZEBITMAP structure and has a cursory initialization from
 * settings in a distance map.
 * returns NULL on error
 */
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
  mb->colorfunc = NULL;
  mb->udata    = NULL;

  return mb;
} /* createmazebitmap() */


/* Caller is responsible for freeing the distance map and user data */
void
freemazebitmap(MAZEBITMAP *mb)
{
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


/* completely fill a cell with a color */
void
fill_cell(png_byte *image, int count, int channels, int twofer, int *colors)
{
   int k, id, offset;
   char *writer;

   for(id = 0; id < count; id ++) {
     offset = id * channels * twofer;
     writer = &(image[offset]);

     for (k = 0; k < channels * twofer; k ++) {
       if(twofer > 1) {
	 if(k % 2) {
	   /* odd bytes get lower 8 bits */
	   writer[k] = colors[k/2] & 0xff;
	 } else {
	   /* even bytes get upper */
	   writer[k] = (colors[k/2] & 0xff00) >> 8;
	 }
       } else {
	 writer[k] = colors[k] & 0xff;
       }
     }
   }
} /* fill_cell */


/* Draw a horizontal or vertical line in a cell a cell,
 * for r1 <= r2 and c1 <= c2. cwide is cell width, channels / twofer
 * used to understand the colors being used. The colors array is one
 * of the ones from the COLORDATA struct (or same format).
 */
/* kinda complicated since trying to accomodate all color/depth pairs */
void
draw_a_line(png_byte *image, int r1, int c1,
                         int r2, int c2,
            int cwide, int channels, int twofer,
	    int *colors)
{
   int i, j, k, id, offset;
   char *writer;
   
   for( i = r1; i <= r2; i ++ ) {
     for( j = c1; j <= c2; j ++ ) {
       id = i * cwide + j;
       offset = id * channels * twofer;
       writer = &(image[offset]);

       for (k = 0; k < channels * twofer; k ++) {
         if(twofer > 1) {
           if(k % 2) {
	     /* odd bytes get lower 8 bits */
	     writer[k] = colors[k/2] & 0xff;
	   } else {
	     /* even bytes get upper */
	     writer[k] = (colors[k/2] & 0xff00) >> 8;
	   }
	 } else {
	   writer[k] = colors[k] & 0xff;
	 }
       }
     }
   }
} /* draw_a_line() */


/* if colorfunc is undefined when default_drawcell is used to draw a maze,
 * this colorpicker routine kicks in.
 */
void
default_colorpicker(MAZEBITMAP *mb, CELL *c, COLORDATA *cd)
{
  int i, percent, adjust;

  /* We're lazy and always put in the alpha, so test gray or not gray */
  if ( cd->channels < 3 ) {
    cd->wall[0] = GRAY_WALL_LINE;
    cd->edge[0] = GRAY_EDGE_LINE;
    cd->fg[0]   = GRAY_FG;
    cd->bg[0]   = GRAY_BG;
    cd->wall[1] = cd->edge[1] = cd->fg[1] = cd->bg[1] = ALPHA_OPAQUE;
    cd->wall[2] = cd->edge[2] = cd->fg[2] = cd->bg[2] =
    cd->wall[3] = cd->edge[3] = cd->fg[3] = cd->bg[3] = 0;
    /* end setting grays */
  } else {
    /* setting not gray */
    cd->wall[0] = R_WALL_LINE;
    cd->edge[0] = R_EDGE_LINE;
    cd->fg[0]   = R_FG;
    cd->bg[0]   = R_BG;
    cd->wall[1] = G_WALL_LINE;
    cd->edge[1] = G_EDGE_LINE;
    cd->fg[1]   = G_FG;
    cd->bg[1]   = G_BG;
    cd->wall[2] = B_WALL_LINE;
    cd->edge[2] = B_EDGE_LINE;
    cd->fg[2]   = B_FG;
    cd->bg[2]   = B_BG;
    cd->wall[3] = cd->edge[3] = cd->fg[3] = cd->bg[3] = ALPHA_OPAQUE;
    /* end setting colors */
  }

  if( cd->depth > 8 ) {
    for(i = 0; i < 4; i++) {
      /* multiplying an 8 bit number by 0x101 works just like
       * multiplying a 2 digit decimal number by 101.
       */
      cd->wall[i] = cd->wall[i] * 0x101;
      cd->edge[i] = cd->edge[i] * 0x101;
      cd->fg[i]   = cd->fg[i]   * 0x101;
      cd->bg[i]   = cd->bg[i]   * 0x101;
    }
  } /* if adjusting for depth */

  /* if a color image, at least depth 8, with a fully calculated distance
   * map and that map has non-trivial distances, then adjust background
   * color by distance.
   */
  if((cd->depth > 7) && (cd->channels > 2) && 
      mb && mb->dmap && (mb->dmap->farthest > 3)) {

    adjust = mb->dmap->map[c->id];
    if (adjust < 2) { adjust = 2; } /* threshold minimum */
    adjust = adjust * 100;
    percent = (adjust / mb->dmap->farthest);

    for(i = 0; i < 4; i++) {
      cd->bg[i]   = (cd->bg[i] * percent) / 100;
    }
    
  } /* if let's vary color with distance */

} /* default_colorpicker */


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
  COLORDATA colors;

  colors.depth    = mb->colordepth;
  colors.channels = mb->channels;
  default_colorpicker(mb, c, &colors);

  fill_cell(image, wid * hei, siz, twofer, &(colors.bg[0]));

  walls = wallstatusbycell(c);
  if(walls & NORTH_WALL) {
    i1 = i2 = 0;
    j1 = 0;
    j2 = wid - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors.wall[0]));
  }
  if(walls & SOUTH_WALL) {
    i1 = i2 = hei - 1;
    j1 = 0;
    j2 = wid - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors.wall[0]));
  }
  if(walls & WEST_WALL) {
    j1 = j2 = 0;
    i1 = 0;
    i2 = hei - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors.wall[0]));
  }
  if(walls & EAST_WALL) {
    j1 = j2 = wid - 1;
    i1 = 0;
    i2 = hei - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors.wall[0]));
  }

  edges = edgestatusbycell(mb->dmap->grid, c);
  if(edges & NORTH_EDGE) {
    i1 = i2 = 0;
    j1 = 0;
    j2 = wid - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors.edge[0]));
  }
  if(edges & SOUTH_EDGE) {
    i1 = i2 = hei - 1;
    j1 = 0;
    j2 = wid - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors.edge[0]));
  }
  if(edges & WEST_EDGE) {
    j1 = j2 = 0;
    i1 = 0;
    i2 = hei - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors.edge[0]));
  }
  if(edges & EAST_EDGE) {
    j1 = j2 = wid - 1;
    i1 = 0;
    i2 = hei - 1;
    draw_a_line(image, i1, j1, i2, j2, wid, siz, twofer, &(colors.edge[0]));
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


/* Draws a maze into the maze bitmap. If the cellfunc is not defined,
 * supplies a default. Then iterates the drawandplacecell() function.
 */
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


/* PNM is a family of easy to read / write file formats that are terrible
 * long term use, but good for easy file interchange between programs.
 * The "N" stans for "aNy".
 *
 * PBM holds bit maps, eight bits to a byte, each row padded out to a
 * a full byte, 1 is black, 0 is white.
 *
 * PGM holds "graymaps" (grayscale) images, with any "maxval" (eg 255 for
 * 8 bit). Each value is stored in 1 byte for 8 or fewer bits, and 2 bytes
 * for 16 or fewer, and in a text numbers separated by whitespace for
 * crazy depths (not implemented here).
 *
 * PPM holds "pixmaps" (RGB) images, in an obvious extension of PGM. Three
 * bytes per pixel for 8 bit or less, 6 for 16 bits.
 *
 * Then there's PAM which is more flexible and can contain basically any
 * rectangular 2D image, at the expense of fewer programs can read it.
 * My installed version of Gimp, for example doesn't recognize it, but my
 * Imagemagick's convert handles it fine. PAM is used for GRAYSCALE_ALPHA
 * and RGB_ALPHA here. PAM has an explicit maxval of 65535, and does not
 * have a text format.
 *
 * The files have a two byte magic number to identify them:
 * "P1" text format PBM, "P2" text format PGM, "P3" text format PGM 
 * "P4" binary format PBM, "P5" binary format PGM, "P6" binary format PGM 
 * "P7" PAM.
 *
 * PBM has the simplest header:
 *	magic-number <newline>
 *	zero or more comment lines marked with leading #
 *	asciiformat-width <whitespace> asciiformat-height <newline>
 *
 * PGM and PPM have a third required value in the header:
 *	magic-number <newline>
 *	zero or more comment lines marked with leading #
 *	asciiformat-width <whitespace> asciiformat-height <newline>
 *	asciiformat-maxvalue <newline>
 *
 * The maxvalue (if present in all channels) represents white. Max up
 * to 255 puts one value per byte, up to 65535 puts one value per two
 * bytes.
 *
 * After the headers, each of those formats just has the binary data dumped.
 * (Or in text versions, ascii numbers with whitespace between them.)
 *
 * PAM is much newer, and a bit more complicated in header, the binary data
 * format is the obvious extension. We'll use either of two PAM headers:
 *      P7
 *      WIDTH asciiformat-width
 *      HEIGHT asciiformat-height
 *      DEPTH 2
 *      MAXVAL asciiformat-maxval
 *      TUPLTYPE GRAYSCALE_ALPHA
 *      ENDHDR
 * or
 *      P7
 *      WIDTH asciiformat-width
 *      HEIGHT asciiformat-height
 *      DEPTH 4
 *      MAXVAL asciiformat-maxval
 *      TUPLTYPE RGB_ALPHA
 *      ENDHDR
 *
 * Where maxval is the largest number storable in that number of bits
 * (1, 3, 15, 255, 65535). And "DEPTH" is what I call "channels".
 *
 * All of these formats are flexible enough to store unusual sample
 * sizes like 12 bit or 0 to 100 for a channel. I don't support them
 * with this writer, though.
 *
 * The user is expected to provide a suitable suffix, if one is desired.
 * A filename of '-' will write to STDOUT instead of a file, useful for
 * pipelines.
 */
int
writepnm(MAZEBITMAP *mb, char *filename)
{
  int maxval;
  int i, j, k, sample;
  png_byte *row;
  FILE *fp;

  if(!mb) { return -1; }

  switch (mb->colordepth) {
    case  1: maxval =     1; break;
    case  2: maxval =     3; break;
    case  4: maxval =    15; break;
    case  8: maxval =   255; break;
    case 16: maxval = 65535; break;
    default: return -1;
  }

  if((filename[0] == '-') && (filename[1] == 0)) {
    int fno = fileno(stdout);
    fp = fdopen(fileno(stdout), "w");
  } else {
    fp = fopen(filename, "w");
  }

  if(!fp) {
    fprintf(stderr, "failed to open %s", filename);
    return -1;
  }

  /* It would cool and all if I ever checked return values from fprintf
   * or putc. Consider it a TODO for "disk full" type scenarios.
   */
  if(mb->channels == 2) {
    /* PAM grayscale */
    fprintf(fp, "P7\n" );
    fprintf(fp, "WIDTH %d\n", mb->img_w );
    fprintf(fp, "HEIGHT %d\n", mb->img_h );
    fprintf(fp, "DEPTH 2\n" );
    fprintf(fp, "MAXVAL %d\n", maxval );
    fprintf(fp, "TUPLTYPE GRAYSCALE_ALPHA\n" );
    fprintf(fp, "ENDHDR\n" );
  } else if (mb->channels == 4) {
    /* PAM RGB */
    fprintf(fp, "P7\n" );
    fprintf(fp, "WIDTH %d\n", mb->img_w );
    fprintf(fp, "HEIGHT %d\n", mb->img_h );
    fprintf(fp, "DEPTH 4\n" );
    fprintf(fp, "MAXVAL %d\n", maxval );
    fprintf(fp, "TUPLTYPE RGB_ALPHA\n" );
    fprintf(fp, "ENDHDR\n" );
  } else if ((mb->channels == 1) && (mb->colordepth == 1)) {
    /* PBM */
     fprintf(fp, "P4\n%d %d\n", mb->img_w, mb->img_h);
  } else {
    /* PGM/PPM */
    int magic;
    if(mb->channels == 1) { magic = 5; } else { magic = 6; }
     fprintf(fp, "P%d\n%d %d\n%d\n", magic, mb->img_w, mb->img_h, maxval);
  }

  if(maxval > 254) {
    /* easy case, just write everything out just as stored for png */
    size_t siz = (size_t)(mb->channels + (mb->channels * mb->doubled));
    for(i = 0; i < mb->img_h; i++) {
      png_byte *row = mb->rowsp[i];
      fwrite(row, siz, (size_t)mb->img_w, fp);
    }
  } else if ((mb->channels == 1) && (mb->colordepth == 1)) {
    /* PBM, also kinda easy, trickiest bit is reversal of white / black
     * from PNG.
     */
    png_byte *invert;

    size_t siz = (size_t)(mb->img_w / 8);
    if(mb->img_w % 8) { siz += 1; }

    invert = (png_byte *)malloc(siz);
    if(!invert) { return -1; }

    for(i = 0; i < mb->img_h; i++) {
      png_byte *row = mb->rowsp[i];
      for(j = 0; j < siz; j++) {
        invert[j] = ~row[j];
      }
      fwrite(invert, (size_t)1, siz, fp);
    }

    free(invert);
  } else {
    /* we need to expand those compacted values */
    int DEBG; /* DEBUG */
    for(i = 0; i < mb->img_h; i++) {
      png_byte *row = mb->rowsp[i];
      j = k = 0;
      while(j < mb->img_w) {
        sample = row[k];
        switch(mb->colordepth) {
	  case 4: putc((sample & 0xf0) >> 4, fp);
	          putc((sample & 0x0f), fp);
		  j += 2;
		  k++;
	          break;

	  case 2: putc((sample & 0xc0) >> 6, fp);
	          putc((sample & 0x30) >> 4, fp);
	          putc((sample & 0x0c) >> 2, fp);
	          putc((sample & 0x03), fp);
		  j += 4;
		  k++;
	          break;

	  case 1: /* Who is going to do this? You'd end up here if
	           * you have a one bit RGB, RGBA, or GA.
		   * The rest of my code isn't likely to work for that.
		   */
	          putc((sample & 0x80) >> 7, fp);
	          putc((sample & 0x40) >> 6, fp);
	          putc((sample & 0x20) >> 5, fp);
	          putc((sample & 0x10) >> 4, fp);
	          putc((sample & 0x08) >> 3, fp);
	          putc((sample & 0x04) >> 2, fp);
	          putc((sample & 0x02) >> 1, fp);
	          putc((sample & 0x01), fp);
		  j += 8;
		  k++;
	}; /* switch */
      } /* for byte in each row */
    } /* for row */
  } /* expanding compacted values */

  fclose(fp);
  return 0;
} /* writepnm() */


/* Write a PNG version of a maze bitmap.
 * This is (currently) the only function that actually uses libpng
 * functions (as opposed to libpng #defines and data types).
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

