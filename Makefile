
CFLAGS = -g

mazes: binary_tree sidewinder

test: testgrid testdistance
	./testgrid
	./testdistance
	@echo
	@echo ALL TESTS SUCCEEDED

clean:
	rm -rf *.o testgrid testdistance core

testgrid: testgrid.o grid.o
testdistance: testdistance.o distance.o grid.o
binary_tree: binary_tree.o grid.o
sidewinder: sidewinder.o grid.o

testgrid.o: grid.h
testdistance.o: distance.h grid.h
binary_tree.o: grid.h
sidewinder.o: grid.h
grid.o: grid.h

