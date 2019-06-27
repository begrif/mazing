Maze coding
-----------

Inspired by _Mazes for Programmers_ by Jamis Buck. That has example code
in Ruby, for ASCII and PNG output.

https://pragprog.com/book/jbmaze/mazes-for-programmers

Programs
--------

Maze demos:

1. `binary_tree`
   * proof of concept maze, rather boring
   * ascii only output
2. `sidewinder`
   * better maze, but still has highly visible artifact
   * ascii only output
3. `aldousbroder` and `aldousbroder_masked`
   * relatively slow random walk algorithm with nice looking mazes
   * Aldous-Broder has one walk that visits every cell
   * ascii only output
   * `aldousbroder` prints a blank and a solved version
   * `aldousbroder_masked` is the same, but with four corners masked off
4. `wilson` and `wilson_masked`
   * relatively slow random walk algorithm with nice looking mazes
   * Wilson has multiple walks that stop when reaching a visited cell
   * ascii only output
   * `wilson` prints a blank and a solved version
   * `wilson_masked` is the same, but with two corners masked off
5. `huntkill` and `huntkill_masked`
   * a random walk that finds a new starting point (slowly) when dead-ended
   * ascii only output
   * `huntkill` prints a blank and a solved version
   * `huntkill_masked` is the same, but with a bottle-shaped mask
6. `backtracker` and `backtracker_masked`
   * a random walk which backtracks to a new branch point when dead-ended
   * ascii only output
   * `backtracker` prints a blank and a solved version
   * `backtracker_masked` is the same, but with a center circle mask

**Note**: The nature of the `sidewinder` and `binarytree` mazes means they
would break with some masks, so no masking implementation is provided.

Code test:

1. testgrid
   * code to test grid.c functions
   * ascii only output
2. testdistance
   * code to test distance.c functions
   * code to test hollow() iterator, written for testdistance
   * ascii only output
3. testmazeimg   
   * code to test maze to image functions
   * makes many color variation versions of serpentine maze
   * PNG and PNM output
4. testmazeimgstdout
   * code to test maze to image writepnm() for standard out case
   * demos a custom drawcell() function
   * PNM output of a single solved maze

In progress:


General code
------------

1. `grid.c` and `grid.h`
   * implements a grid with a notion of walls between cells
   * provides an ASCII art grid printer
   * TODO: building walls (deleting connections)
2. `distance.c` and `distance.h`
   * as an adjuct to `grid.c`, this measures distances
   * finds one shortest path (just one, even if multiple are possible)
   * finds one longest path (just one, even if multiple are possible)
   * not constrained to particular maze topologies
3. `mazes.c` and `mazes.h`
   * implementations of grid-to-maze methods
   * divided into two broad classes, iterategrid() callbacks and others
4. `mazeimg.c` and `mazeimg.h`
   * maze to image functions
   * draws cells are raw bitmaps, stores whole maze image as raw PNG input
   * PNG output for RGBA, RGB, GA (gray + alpha) in either 8 or 16 bits
   * PNG output for Gray in 1, 2, 4, 8, or 16 bits
   * PNM (PBM/PGM/PPM/PAM) for all PNG supported color and grayscale depths

Short variables by convention:
 * `g` is grid
 * `c` is a cell
 * `i` is a row number
 * `j` is a column number
 * `t` is a gtype/ctype
 * `d` is a direction or a distance
 * `go` is a direction
 * `dm` is a distance map
 * `rc` is a return code
 * `mb` is a maze bitmap structure


Author
------
This implementation by Benjamin Elijah Griffin / Eli the Bearded.
Begun 11 June 2019.

