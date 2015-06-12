#pragma once
#ifndef __PACMAN__
#define __PACMAN__
#include <algorithm>
typedef std::pair<int,int> Coord;
#define x first
#define y second

class PacMan {
private:
	int x, y, sx, sy, lives;
public:
	PacMan(int lives = 3):
		x(0), y(0), sx(0), sy(0), lives(lives) {}
	inline Coord position(void) const {
		return {x, y};
	}
	inline void move(int dx, int dy) {
		x += dx;
		y += dy;
	}
	inline int lives_left(void) {
		return lives;
	}
	inline void set_dead(void) {
		lives -= 1;
		x = sx;
		y = sy;
	}
};
#undef x
#undef y
#endif