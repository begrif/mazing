
CFLAGS = -g

test: testgrid
	./testgrid

clean:
	rm -rf *.o testgrid core

testgrid: testgrid.o grid.o

testgrid.o: grid.h
grid.o: grid.h

