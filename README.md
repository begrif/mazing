Maze coding
-----------

Inspired by _Mazes for Programmers_ by Jamis Buck. That has example code
in Ruby, for ASCII and PNG output.

https://pragprog.com/book/jbmaze/mazes-for-programmers

Programs
--------

Maze demos:

1. binary_tree
   * proof of concept maze, rather boring
   * ascii only output
2. sidewinder
   * better maze, but still has highly visible artifact
   * ascii only output
3. aldousbroder
   * relatively slow random walk algorithm with nice looking mazes
   * Aldous-Broder has one walk that visits every cell
   * ascii only output
   * prints a blank and a solved version
4. wilson
   * relatively slow random walk algorithm with nice looking mazes
   * Wilson has multiple walks that stop when reaching a visited cell
   * ascii only output
   * prints a blank and a solved version

Code test:

1. testgrid
   * code to test grid.c functions
   * ascii only output
2. testdistance
   * code to test distance.c functions
   * ascii only output
   * TODO: needs better test cases for longest path solving

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


Author
------
This implementation by Benjamin Elijah Griffin / Eli the Bearded.
Begun 11 June 2019.

