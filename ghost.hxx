#pragma once
#ifndef __GHOST__
#define __GHOST__
#include<vector>
#include<random>
#include<algorithm>
#include"random.hxx"
#include"graphics.hxx"
#define x first
#define y second

class Ghost {
private:
	typedef std::pair<int,int> Coord;
	typedef std::uniform_real_distribution<> RealDistribution;

	int own_color;
	int x, y, sx, sy;
	vector<Coord> tabu_list;
	long long edible_until, dead_until;
	Random<RealDistribution, double> real;

	void operator = (Ghost const&) = delete;
	Ghost(Ghost const&) = delete;

public:
	Ghost& operator = (Ghost&&) { return *this; }

	Ghost(int x, int y, int own_color, int tabu_size = 3):
		sx(x), sy(y), x(x), y(y),
		edible_until(-1LL), dead_until(-1LL), own_color(own_color),
		tabu_list(tabu_size, {-1, -1}), real(0., 1.) {}

	Ghost(Ghost&& r):
		sx(r.x), sy(r.y), x(r.x), y(r.y),
		edible_until(r.edible_until), dead_until(r.dead_until), own_color(r.own_color),
		tabu_list(r.tabu_list), real(0., 1.) {}

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
	inline bool move(long long current_time, int dx, int dy, bool random = true) {
		static const double threshold = 0.60;
		// ghost is slower than the pacman.
		if (current_time % 7 == 0 && !dead(current_time) &&
			 (real.GetRandom() > threshold ||
			 find(tabu_list.begin(), tabu_list.end(), Coord(x+dx, y+dy)) ==
			 tabu_list.end())) {
			x += dx;
			y += dy;
			tabu_list.push_back({x, y});
			return true;
		} else {
			return false;
		}
	}
};
#endif
