Maze coding
-----------

Inspired by _Mazes for Programmers_ by Jamis Buck. That has example code
in Ruby, for ASCII and PNG output.

https://pragprog.com/book/jbmaze/mazes-for-programmers

Programs
--------

1. binary_tree
   * proof of concept maze, rather boring
   * ascii only output
2. sidewinder
   * better maze, but still has highly visible artifact
   * ascii only output
3. testgrid
   * code to test grid.c functions
   * ascii only output
3. testdistance
   * code to test distance.c functions
   * ascii only output

General code
------------

1. `grid.c` and `grid.h`
   * implements a grid with a notion of walls between cells
   * provides an ASCII art grid printer
   * TODO: building walls (deleting connections)
2. `distance.c` and `distance.h`
   * as an adjuct to `grid.c`, this measures distances
   * finds one shortest path (even if multiple are possible)
   * not constrained to particular maze topologies

Author
------
This implementation by Benjamin Elijah Griffin / Eli the Bearded.
Begun 11 June 2019.

