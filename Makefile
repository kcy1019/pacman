CC=g++
CFLAGS=-O3 -std=c++11
LFLAGS=-lncurses

all:
	$(CC) -o pacman $(CFLAGS) main.cc $(LFLAGS)

trainer:
	$(CC) -o trainer $(CFLAGS) trainer.cc $(LFLAGS)

autoplay:
	$(CC) -o autoplay $(CFLAGS) autoplay.cc $(LFLAGS)

clean:
	rm -f pacman trainer autoplay disjointset.gch game.gch\
		ghost.gch graphics.gch pacman.gch random.gch timer.gch\
		simple.genetic.hxx.gch game.nogui.hxx.gch
