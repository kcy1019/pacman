CC=g++
CFLAGS=-o pacman -O3 -std=c++11
LFLAGS=-lncurses

all:
	$(CC) $(CFLAGS) main.cc $(LFLAGS)

genetic:
	$(CC) $(CFLAGS) trainer.cc $(LFLAGS)

autoplay:
	$(CC) $(CFLAGS) autoplay.cc $(LFLAGS)

clean:
	rm -f pacman disjointset.gch game.gch ghost.gch graphics.gch\
		pacman.gch random.gch timer.gch genetic.game.gch
