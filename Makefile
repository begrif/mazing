
CFLAGS = -g

mazes: binary_tree sidewinder

test: testgrid
	./testgrid

clean:
	rm -rf *.o testgrid core

testgrid: testgrid.o grid.o
binary_tree: binary_tree.o grid.o
sidewinder: sidewinder.o grid.o

testgrid.o: grid.h
binary_tree.o: grid.h
sidewinder.o: grid.h
grid.o: grid.h

