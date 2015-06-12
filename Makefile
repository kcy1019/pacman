CC=g++
CFLAGS=-o pacman -O3 -std=c++11 -lncurses

all:
	$(CC) main.cc $(CFLAGS)

clean:
	rm -f pacman disjointset.gch game.gch ghost.gch graphics.gch pacman.gch random.gch timer.gch
