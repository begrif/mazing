/* July 2019, Benjamin Elijah Griffin / Eli the Bearded */

#ifndef _FORFOUR_H
#define _FORFOUR_H

#include "grid.h"
#include "distance.h"
#include "mazeimg.h"

#define SOLVEDCELL   23
#define MAYBE        (3*NC)

typedef char color_overide_t[8][8];

int marksolved(DMAP *, int /* cid */, void * /* unused */);
int verifycolor(char */* color */);
int hexpair(char, char);
int proffir(char);
void copycolor(color_overide_t /* usercolors */, char, char *);
COLORDATA *colors(color_overide_t /* usercolors */);
int cleandraw(MAZEBITMAP *, png_byte *, CELL *);

#endif /* _FORFOUR_H */
