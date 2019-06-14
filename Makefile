
CFLAGS = -g

allmazes: binary_tree sidewinder aldousbroder

test: testgrid testdistance
	./testgrid
	./testdistance
	@echo
	@echo ALL TESTS SUCCEEDED

clean:
	rm -rf *.o testgrid testdistance core

testgrid: testgrid.o grid.o
testdistance: testdistance.o distance.o grid.o mazes.o
binary_tree: binary_tree.o grid.o mazes.o
sidewinder: sidewinder.o grid.o mazes.o
aldousbroder: aldousbroder.o distance.o grid.o mazes.o

mazes.o: distance.h grid.h mazes.h
testgrid.o: grid.h
testdistance.o: distance.h grid.h mazes.h
binary_tree.o: grid.h mazes.h
sidewinder.o: grid.h mazes.h
grid.o: grid.h mazes.h

