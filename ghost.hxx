#pragma once
#ifndef __GHOST__
#define __GHOST__
#include<algorithm>
#include"graphics.hxx"
typedef std::pair<int,int> Coord;
#define x first
#define y second

class Ghost {
private:
	int x, y, sx, sy;
	long long edible_until, dead_until;
public:
	int own_color;
	Ghost(int x, int y, int own_color):
		sx(x), sy(y), x(x), y(y),
		edible_until(-1LL), dead_until(-1LL), own_color(own_color) {}
	inline int color(long long current_time) const {
		return edible(current_time) ? GraphicsToolkit::WHITE :
			   dead(current_time) ? GraphicsToolkit::GRAY :
			   own_color;
	}
	inline Coord position(void) const {
		return {x, y};
	}
	inline bool edible(long long t) const {
		return t <= edible_until;
	}
	inline bool dead(long long t) const {
		return t <= dead_until;
	}
	inline void set_edible(long long until) {
		edible_until = until;
	}
	inline void set_dead(long long until) {
		x = sx;
		y = sy;
		dead_until = until;
		edible_until = -1;
	}
	inline void move(long long current_time, int dx, int dy, bool random = true) {
		static int last_dx = -100, last_dy = -100;
		// ghost is slower than the pacman.
		if (current_time % 7 == 0 &&
			!dead(current_time) &&
			(!random ||
			 (-last_dx != dx ||
			  -last_dy != dy))) {
			x += dx;
			y += dy;
			last_dx = dx;
			last_dy = dy;
		}
	}
};
#endif
