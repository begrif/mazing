
CFLAGS = -g -std=c99 -D_BSD_SOURCE
LDLIBS = -lpng

GAMES = btadventure four

ALLMAZES = binary_tree sidewinder aldousbroder wilson huntkill backtracker \
	aldousbroder_masked wilson_masked huntkill_masked backtracker_masked 

TESTPROGRAMS = testgrid testdistance testmazeimg testmazeimgstdout

all: allgames allmazes testprograms

allgames: $(GAMES)

allmazes: $(ALLMAZES)

testprograms: $(TESTPROGRAMS)

test: testprograms
	./testgrid
	./testdistance
	./testmazeimg
	./testmazeimgstdout > tmp-testmazeimgstdout.pnm
	@echo Capturing header and first 4 pixels of random maze
	head -c 27 tmp-testmazeimgstdout.pnm > tmp-tmiso-head.pnm 
	@echo Capturing last 4 pixels of random maze
	tail -c 12 tmp-testmazeimgstdout.pnm > tmp-tmiso-tail.pnm 
	@echo Checksumming all non-random mazes and random maze fragments.
	md5sum -c mazeimg-test.checksums
	@echo
	@echo ALL TESTS SUCCEEDED

clean:
	rm -rf *.o tmp*.png tmp*.pnm four-default-*.png core core.[0-9]*

realclean: clean
	rm -f $(GAMES) $(ALLMAZES) $(TESTPROGRAMS)

btadventure: btadventure.o grid.o mazes.o
four: four.o forfour.o grid.o distance.o mazes.o mazeimg.o

testgrid: testgrid.o grid.o
testdistance: testdistance.o distance.o grid.o mazes.o
testmazeimg: testmazeimg.o mazeimg.o distance.o grid.o mazes.o
testmazeimgstdout: testmazeimgstdout.o mazeimg.o distance.o grid.o mazes.o
binary_tree: binary_tree.o grid.o mazes.o
sidewinder: sidewinder.o grid.o mazes.o
aldousbroder: aldousbroder.o distance.o grid.o mazes.o
aldousbroder_masked: aldousbroder_masked.o distance.o grid.o mazes.o
wilson: wilson.o distance.o grid.o mazes.o
wilson_masked: wilson_masked.o distance.o grid.o mazes.o
huntkill: huntkill.o distance.o grid.o mazes.o
huntkill_masked: huntkill_masked.o distance.o grid.o mazes.o
backtracker: backtracker.o distance.o grid.o mazes.o
backtracker_masked: backtracker_masked.o distance.o grid.o mazes.o

mazes.o: distance.h grid.h mazes.h
testgrid.o: grid.h
grid.o: grid.h mazes.h
testdistance.o: distance.h grid.h mazes.h
distance.o: distance.h
mazeimg.o: mazeimg.h distance.h grid.h
testmazeimg.o: mazeimg.h mazes.h distance.h grid.h
testmazeimgstdout.o: mazeimg.h mazes.h distance.h grid.h
btadventure.o: grid.h mazes.h
four.o: forfour.h mazeimg.h mazes.h distance.h grid.h
forfour.o: forfour.h mazeimg.h distance.h grid.h
binary_tree.o: grid.h mazes.h
sidewinder.o: grid.h mazes.h
aldousbroder.o: grid.h mazes.h distance.h
aldousbroder_masked.o: grid.h mazes.h distance.h
wilson.o: grid.h mazes.h distance.h
wilson_masked.o: grid.h mazes.h distance.h
huntkill.o: grid.h mazes.h distance.h
huntkill_masked.o: grid.h mazes.h distance.h
backtracker.o: grid.h mazes.h distance.h
backtracker_masked.o: grid.h mazes.h distance.h

