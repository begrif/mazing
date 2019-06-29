/* June 2019, Benjamin Elijah Griffin / Eli the Bearded */

/* Build a maze, find dead ends, put things there, then have a
 * player collect those things, text adventure style. Uses the
 * distinctive long corridors of a binary tree as an opportunity
 * for naming rooms.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "grid.h"
#include "mazes.h"

#define CELL_NORM  1
#define CELL_START 2
#define CELL_HALL  3
#define CELL_PRIZE 4
#define CELL_TRAP  5

typedef struct roomdata_s {
  int visits;
} ROOMDATA;

int
longnamer(GRID *notused, CELL *c, void *unused)
{
  if(c->ctype == CELL_NORM) {
    c->ctype = CELL_HALL;
    namebycell(c, "Long corridor"); 
  }
  return 0;
} /* longnamer() */

int
markprizes(GRID *notused, CELL *c, void *unused)
{
  ROOMDATA *room;
  int missed = 0;

  if(c->ctype == CELL_TRAP) {
    namebycell(c, "Trap"); 
  } else if(c->ctype == CELL_PRIZE) {
    room = (ROOMDATA*)c->data;
    if(room->visits < 0) {
      namebycell(c, "PRIZE"); 
      missed ++;
    } else {
      namebycell(c, "Found"); 
    }
  } else if(c->ctype != CELL_START) {
    namebycell(c, " "); 
  }
  return missed;
} /* markprizes() */

int
prizer(GRID *g, CELL *c, void *unused)
{
  int prize = 0;
  ROOMDATA *room;

  c->data = malloc(sizeof(ROOMDATA));
  room = (ROOMDATA*)c->data;
  room->visits = -1;

  if(c->ctype == CELL_NORM) {
    if(1 == ncountbycell(g, c, EXITS, 0)) {
      /* only want dead ends, ie 1 exit */
      c->ctype = CELL_PRIZE;
      namebycell(c, "Cache site"); 
      prize = 1;
    } /* dead ends */
    else {
      namebycell(c, "Unremarkable room"); 
    }
  } /* CELL_NORM */
  return prize;
} /* prizer() */

GRID *
createdungeon(void)
{
  GRID *g;
  CELL *c;
  int  deg;

  g = creategrid(4,4,CELL_NORM);

  if(!g) { return NULL; }

  iterategrid(g, btreewalker, NULL);

  c = visitrc(g, 0, g->cols -1);

  /* Our start */
  namebycell(c, "Entrance");
  c->ctype = CELL_START;

  /* The major halls */
  iteraterow(g, 0, longnamer, NULL);
  iteratecol(g, g->cols - 1, longnamer, NULL);

  /* The remaining rooms */
  g->gtype = iterategrid(g, prizer, NULL);

  /* rotate takes 90, 180, and 270 as acceptable values for
   * CLOCKWISE, ROTATE_180, and COUNTERCLOCKWISE
   */
  deg = 90 * (1 + (random() % 3));
  if(rotategrid(g, deg)) {
    printf("rotation failed\n");
    return(NULL);
  }

  return g;
} /* createdungeon() */

void
showdungeon(GRID *g)
{
  char *board;
  int notfound;

  notfound = iterategrid(g, markprizes, NULL);
  printf("The bunker with %d unfound prize%s of %d total:\n\n", 
  		notfound, (notfound != 1)? "s": "",  g->gtype);
  board = ascii_grid(g, 1);
  puts(board);
  free(board);

  printf("\nKey\n");
  printf("PRI: unfound prize\n");
  printf("Fou: looted prize crate\n");
  printf("Ent: entrance\n");
  printf("Tra: deadly trap\n");

} /* showdungeon() */


int
showroom(GRID *g, CELL *c)
{
  ROOMDATA *room;
  int score = 0;

  room = (ROOMDATA*)c->data;
  room->visits ++;

  printf("%s\n\n", c->name);
  switch(c->ctype) {
    case CELL_START:
	if(room->visits < 1) {
	  printf("You jump down from the end of the rope and look around\n");
	  printf("the underground bunker.\n\n");
	} else {
	  printf("There's a rope here to exit up.\n");
	}
	printf("You can see two long hallways that meet at this point.\n");
	break;

    case CELL_HALL:
	printf("This is a long underground hallway.\n");
	break;

    case CELL_PRIZE:
	if(room->visits < 1) {
	  score ++;
	  printf("There's a large crate here. You eagerly smash it open and\n");
	  printf("collect the shiny prize within. Nice.\n\n");
	  
	} else {
	  printf("There is the remains of a large crate here.\n");
	}
	break;

    default:
	printf("This is a musty corridor.\n");
	break;
  } /* switch */

  if(room->visits == 1) {
    printf("\nYou can see by footprints in the dust you've been here before.\n");
  } else if (room->visits > 1) {
    printf("\nThere are several sets of footprints here.\n");
  }

  return score;
} /* showroom() */


CELL *
visitnext(GRID *g, CELL *c)
{
  CELL *to_n, *to_s, *to_e, *to_w;
  ROOMDATA *room;
  char buffer[BUFSIZ];
  int comma, i, retry = 0;

  room = (ROOMDATA*)c->data;

  if((room->visits > 5) && (random() % 3)) {
    printf("Tramping through this room yet again, you disturb an old vibration\n");
    printf("sensor. The pins holding up the concrete ceiling retract with fatal\n");
    printf("consequences.\n");
    c->ctype = CELL_TRAP;
    return NULL;
  }

  to_n = visitdir(g, c, NORTH, THIS);
  to_s = visitdir(g, c, SOUTH, THIS);
  to_w = visitdir(g, c, WEST,  THIS);
  to_e = visitdir(g, c, EAST,  THIS);

  while(retry < 3) {
    comma = 0;
    printf("From here you can exit");

    if(to_n) {                   printf("%s north", comma? "," : ""); comma ++; }
    if(to_s) {                   printf("%s south", comma? "," : ""); comma ++; }
    if(to_w) {                   printf("%s west",  comma? "," : ""); comma ++; }
    if(to_e) {                   printf("%s east",  comma? "," : ""); comma ++; }
    if(c->ctype == CELL_START) { printf("%s up",    comma? "," : ""); }
    printf(".\n\nWhich way? ");

    fgets(buffer, BUFSIZ, stdin);
    
    for(i = 0; i < BUFSIZ; i++) {
      switch( buffer[i] ) {
	case 0:
	case '\n': i = BUFSIZ; break;

	case 'q':
	case 'Q': printf("You decide to quit the game.\n\n");
	          return NULL;
		  break;

	case 'n':
	case 'N': if(to_n) { printf("\n"); return to_n; } else { retry ++; }
		  break;

	case 's':
	case 'S': if(to_s) { printf("\n"); return to_s; } else { retry ++; }
		  break;

	case 'w':
	case 'W': if(to_w) { printf("\n"); return to_w; } else { retry ++; }
		  break;

	case 'e':
	case 'E': if(to_e) { printf("\n"); return to_e; } else { retry ++; }
		  break;

	case 'u':
	case 'U': if(c->ctype == CELL_START) {
		    printf("You ensure all the prizes are securely in your pack and\n");
		    printf("jump up. Hand over hand, you climb up and out.\n\n");
		    return NULL;
		  } else {
		    if(retry < 2) {
		      printf("You jump up, but there's no point really.\n\n");
		      retry ++;
		    } else {
		      printf("You jump up, but when you land the thin floor breaks. You plunge\n");
		      printf("into a pit filled with a large number of spears with fatal\n");
		      printf("consequences.\n");
		      c->ctype = CELL_TRAP;
		      return NULL;
		    }
		  }
		  break;
      } /* switch */
    }

  } /* loop forever */

  printf("While wandering aimlessly in this room you find a hidden tripline.\n");
  printf("Nearly instantly several spears spring up from the floor with fatal\n");
  printf("consequences.\n");
  c->ctype = CELL_TRAP;
  return NULL;

} /* visitnext() */


int
playdungeon(GRID *g)
{
  int score = 0;
  CELL *c;

  /* Find the start cell by checking all four corners. */
  c = visitrc(g, g->rows - 1, g->cols - 1);

  if(c->ctype != CELL_START) {
    c = visitrc(g, 0, 0);

    if(c->ctype != CELL_START) {
      c = visitrc(g, g->rows - 1, 0);

      if(c->ctype != CELL_START) {
	c = visitrc(g, 0, g->cols - 1);

        if(c->ctype != CELL_START) {
	  printf("Failed to find start!\n");
	  return 0;
	}
      }
    }
  }

  while (1) {
    score += showroom(g, c);
    c = visitnext(g, c);
    if(!c) {
      return score;
    }
  }

} /* playdungeon() */

int
main(int notused, char**ignored)
{
  GRID *g;
  int score;

  g = createdungeon();

  score = playdungeon(g);
  printf("Final score: %d out of a possible %d\n", score, g->gtype);

  showdungeon(g);
  freegrid(g);
  return 0;
}
