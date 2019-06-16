
CFLAGS = -g

allmazes: binary_tree sidewinder aldousbroder wilson huntkill backtracker

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
wilson: wilson.o distance.o grid.o mazes.o
huntkill: huntkill.o distance.o grid.o mazes.o
backtracker: backtracker.o distance.o grid.o mazes.o

mazes.o: distance.h grid.h mazes.h
testgrid.o: grid.h
grid.o: grid.h mazes.h
testdistance.o: distance.h grid.h mazes.h
distance.o: distance.h

binary_tree.o: grid.h mazes.h
sidewinder.o: grid.h mazes.h
aldousbroder.o: grid.h mazes.h distance.h
wilson.o: grid.h mazes.h distance.h
huntkill.o: grid.h mazes.h distance.h
backtracker.o: grid.h mazes.h distance.h

