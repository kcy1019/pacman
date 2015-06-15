#pragma once
#ifndef __GENETIC_GAME__
#define __GENETIC_GAME__
#include<vector>
#include<random>
#include<string>
#include<climits>
#include<cassert>
#include<fstream>
#include<algorithm>
#include"maze.hxx"
#include"ghost.hxx"
#include"random.hxx"
#include"pacman.hxx"
#define x first
#define y second 
using std::string;
using std::vector;
using std::swap;

static Random<std::uniform_real_distribution<>, double> zero_to_one(0., 1.);
static Random<std::uniform_int_distribution<>, int> rand4(0, 3);
static const int SMALLEST = INT_MIN;

#ifdef __TRAINER__
class Cell {
public:
	char type;
	static const char BISCUIT = 0;
	static const char EMPTY = 1;
	static const char FRUIT = 2;
	static const char WALL = 3;
	Cell(char type): type(type) {}
	inline const bool operator == (const Cell& rhs) const {
		return type == rhs.type;
	}
	inline const bool operator != (const Cell& rhs) const {
		return type != rhs.type;
	}
};
#endif

// Game without GUI(+ncurses) and messages.
// This can play multiple games on the same board.
class TrainingGame {
protected:
	int height, width, score, sight,
		nghosts, nbiscuits, nfruits,
		_nfruits, _nbiscuits;
	vector<vector<Cell>> field, _field;
	vector<Ghost> ghosts;
	PacMan pacman;
	bool finished, timeout;

public:
	static const enum {
		ESC = -1, NONE = 0,
		LEFT = 1, RIGHT = 2, UP = 3, DOWN = 4,
	} ControlSet;

	TrainingGame(int height = 20, int width = 20, int sight = 10, int nghosts = 5):
		height(height), width(width), score(0),
		pacman(), sight(sight), nghosts(nghosts),
		_nbiscuits(0), _nfruits(0), ghosts()
	{
		static const double fruit_dist = 0.90;
		MazeFactory<Cell> factory;
		_field = factory.GenerateMaze(width, height);

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				if (_field[i][j] == Cell::EMPTY) {
					_field[i][j] = Cell::BISCUIT;
					++_nbiscuits;
				} else continue;
				if (i-1 >= 0 && j-1 >= 0 && i+1 < height && j+1 < width &&
					_field[i+1][j] != Cell::WALL &&
					_field[i-1][j] != Cell::WALL &&
					_field[i][j+1] != Cell::WALL &&
					_field[i][j-1] != Cell::WALL &&
					zero_to_one.GetRandom() > fruit_dist) {
					_field[i][j] = Cell::FRUIT;
					++_nfruits;
					--_nbiscuits;
				}
			}
		} 

		Reset();
	}

	inline void Reset(void)
	{
		field.clear();
		field = _field;
		nbiscuits = _nbiscuits;
		nfruits = _nfruits;
		score = 0;
		finished = false;
		timeout = false;
		pacman = PacMan();
		ghosts.clear();

		static const int colors[] = {GraphicsToolkit::RED, GraphicsToolkit::PINK, GraphicsToolkit::CYAN,
									GraphicsToolkit::YELLOW, GraphicsToolkit::GREEN};

		bool escape_loop = false;
		for (int i = height - 1; i >= 0 && !escape_loop; i--) {
			for (int j = width - 1; j >= 0 && !escape_loop; j--) {
				if (field[i][j] != Cell::WALL) {
					int sz_ghost = ghosts.size();
					ghosts.emplace_back(Ghost(j, i, colors[sz_ghost % 5]));
					if (ghosts.size() == nghosts)
						escape_loop = true;
				}
			}
		}
	}

	inline int get_score(void) {
		return score;
	}

	inline bool is_finished(void) {
		return finished;
	}

	inline bool is_timed_out(void) {
		return timeout;
	}

	inline bool Blocked(int x, int y, bool block_overlap = false) const {
		assert(field.size() == height && field[0].size() == width);
		bool chk = !(x >= 0 && y >= 0 && x < width && y < height &&
					field[y][x] != Cell::WALL);
		if (chk || !block_overlap) return chk;
		for (const auto& ghost: ghosts) {
			auto gp = ghost.position();
			if (gp.x == x && gp.y == y)
				return true;
		}
		return chk;
	}

	static inline int Distance(int x, int y, int nx, int ny) {
		return abs(x - nx) + abs(y - ny);
	}

	template<typename InputStream>
	inline void Process(long long current_time, const InputStream& stream)
	{
		static const int dx[4] = {-1, 0, 1, 0},
						 dy[4] = {0, 1, 0, -1};
		int x = pacman.position().x,
			y = pacman.position().y;

		for (auto& ghost: ghosts) {
			if (ghost.dead(current_time)) continue;
			if (ghost.position() == pacman.position()) {
				if (ghost.edible(current_time)) {
					ghost.set_dead(current_time + 100);
					score += 100;
				} else {
					if (pacman.lives_left()) {
						pacman.set_dead();
					} else {
						EndGame();
					}
				}
			}
		}

		for (auto& ghost: ghosts) {
			int gx = ghost.position().x,
				gy = ghost.position().y,
				cur_distance = Distance(gx, gy, x, y);
			bool moved = false, is_random = true;;
			if (cur_distance <= sight) {
				for (int k = 0; k < 4; k++) {
					int nx = gx + dx[k],
						ny = gy + dy[k],
						nxt_distance = Distance(nx, ny, x, y);
					if (!Blocked(nx, ny, true) &&
						(ghost.edible(current_time) ? nxt_distance > cur_distance :
													  nxt_distance < cur_distance)) {
						if (ghost.move(current_time, dx[k], dy[k], false)) {
							moved = true;
							break;
						}
					}
				}
				if (!moved) {
					is_random = false;
				}
			}
			if (!moved) {
				for (int t = 10; t --> 0;) {
					int k = rand4.GetRandom();
					int nx = gx + dx[k],
						ny = gy + dy[k];
					if (!Blocked(nx, ny, true)) {
						if (ghost.move(current_time, dx[k], dy[k], is_random)) {
							break;
						}
					}
				}
			}
		}

		if (field[y][x] == Cell::BISCUIT) {
			field[y][x] = Cell::EMPTY;
			score += 1;
			if (--nbiscuits == 0) {
				// Victory!
				score += (1 + pacman.lives_left()) << 11;
				score += nfruits << 8;
				EndGame();
			}
		} else if (field[y][x] == Cell::FRUIT) {
			field[y][x] = Cell::EMPTY;
			score += 50;
			for (auto& ghost: ghosts) {
				ghost.set_edible(current_time + 500);
			}
		}

		switch(stream(field, ghosts, pacman)) {
			case LEFT:
				if (!Blocked(x-1, y))
					pacman.move(-1, 0);
				break;
			case RIGHT:
				if (!Blocked(x+1, y))
					pacman.move(1, 0);
				break;
			case UP:
				if (!Blocked(x, y-1))
					pacman.move(0, -1);
				break;
			case DOWN:
				if (!Blocked(x, y+1))
					pacman.move(0, 1);
				break;
			case ESC:
				// In case of AI controller, this can be 'timeout'.
				timeout = true;
				EndGame();
		}
	}

	inline void EndGame(void) {
		finished = true;
	}

};

class PacManGene {
protected:
	vector<double> weights;
	int kernel_size, kernel_size_sq, fitness;
	bool timed_out;
public:
	PacManGene(int kn_sz = 5):
		kernel_size(kn_sz), 
		kernel_size_sq(kn_sz * kn_sz),
		weights(kn_sz * kn_sz * 4),
		fitness(SMALLEST),
		timed_out(false)
	{
		// EMPTY CELL WEIGHTS
		int base = 0;
		for (int i = 0; i < kernel_size_sq; i++) {
			weights[base + i] = zero_to_one.GetRandom();
		}
		// BISCUIT CELL WEIGHTS
		base += kernel_size_sq;
		for (int i = 0; i < kernel_size_sq; i++) {
			weights[base + i] = zero_to_one.GetRandom() * 30;
		}
		// FRUIT CELL WEIGHTS
		base += kernel_size_sq;
		for (int i = 0; i < kernel_size_sq; i++) {
			weights[base + i] = zero_to_one.GetRandom() * 100;
		}
		// GHOST CELL WEIGHTS
		base += kernel_size_sq;
		for (int i = 0; i < kernel_size_sq; i++) {
			weights[base + i] = zero_to_one.GetRandom() * -300;
		}
	}

	PacManGene(int kn_sz, const vector<double>& ws):
		weights(ws.begin(), ws.end()),
		kernel_size(kn_sz),
		kernel_size_sq(kn_sz * kn_sz),
		fitness(SMALLEST),
		timed_out(false) {}

	PacManGene(const PacManGene& pmg):
		weights(pmg.weights.begin(), pmg.weights.end()),
		kernel_size(pmg.kernel_size),
		kernel_size_sq(pmg.kernel_size_sq),
		fitness(pmg.fitness),
		timed_out(pmg.timed_out) {}

	PacManGene(const PacManGene&& pmg):
		weights(pmg.weights.begin(), pmg.weights.end()),
		kernel_size(pmg.kernel_size),
		kernel_size_sq(pmg.kernel_size_sq),
		fitness(pmg.fitness),
		timed_out(pmg.timed_out) {}

	inline void operator = (const PacManGene& pmg) {
		weights = pmg.weights;
		kernel_size = pmg.kernel_size;
		kernel_size_sq = pmg.kernel_size_sq;
		fitness = pmg.fitness;
		timed_out = pmg.timed_out;
	}

	inline bool operator < (const PacManGene& pmg) const {
		return fitness > pmg.fitness;
	}

	inline int get_kernel_size(void) const {
		return kernel_size;
	}

	inline const vector<double>& get_weights(void) const {
		return (const vector<double>&)weights;
	}

	inline int size(void) const {
		return (int)weights.size();
	}

	inline double& operator [] (int idx) {
		return weights[idx];
	}

	inline void set_fitness(int fn) {
		fitness = fn;
	}

	inline int get_fitness(void) const {
		return fitness;
	}

	inline void set_timed_out(bool to) {
		timed_out = to;
	}

	inline bool is_timed_out(void) const {
		return timed_out;
	}

	inline void ExportWeights(const string& filename) const {
		std::fstream fs(filename, std::ios::out);
		for (const auto& weight: weights) {
			fs << weight << " ";
		}
		fs.close();
	}

	inline void ImportWeights(const string& filename) {
		std::fstream fs(filename, std::ios::in);
		for (int i = 0; i < weights.size(); i++) {
			fs >> weights[i];
		}
		fs.close();
	}

	inline double EvaluateKernel(int x, int y,
			const vector<vector<Cell>>& field,
			const vector<Ghost>& ghosts,
			const PacMan& pacman) const {
		double score = 0;
		int px, py, gn = ghosts.size(), gx[gn], gy[gn];
		const int gbase = kernel_size_sq * 3;
		for (int i = 0; i < ghosts.size(); i++) {
			std::tie(gx[i], gy[i]) = ghosts[i].position();
		}
		std::tie(px, py) = pacman.position();
		for (int ky = 0; ky < kernel_size; ky++) {
			int h = ky - (kernel_size/2) + y;
			if (h < 0 || h >= field.size())
				continue;
			for (int kx = 0; kx < kernel_size; kx++) {
				int w = kx - (kernel_size/2) + x;
				if (w < 0 || w >= field[0].size() ||
					px == w && py == h)
					continue;

				bool ghost_in_cell = false;
				for (int g = 0; g < gn; g++) {
					if (w == gx[g] && h == gy[g]) {
						score += weights[gbase + (ky * kernel_size + kx)];
						ghost_in_cell = true;
						break;
					}
				}

				if (ghost_in_cell)
					continue;

				int base = 0;
				if (field[h][w] == Cell::EMPTY) {
					score += weights[(ky * kernel_size + kx)];
					continue;
				}
				base += kernel_size_sq;
				if (field[h][w] == Cell::BISCUIT) {
					score += weights[base + (ky * kernel_size + kx)];
					continue;
				}
				base += kernel_size_sq;
				if (field[h][w] == Cell::FRUIT) {
					score += weights[base + (ky * kernel_size + kx)];
					continue;
				}
			}
		}

		return score;
	}

	inline bool Blocked(int x, int y,
			const vector<vector<Cell>>& field) const {
		return !(x >= 0 && y >= 0 &&
				x < field[0].size() && y < field.size() &&
				field[y][x] != Cell::WALL);
	}

	inline int DecideMove(
			const vector<vector<Cell>>& field,
			const vector<Ghost>& ghosts,
			const PacMan& pacman) const {
		static const int dx[] = {-1, 0, 1, 0},
						 dy[] = {0, 1, 0, -1};
		static const int moves[] = {
			TrainingGame::LEFT, TrainingGame::DOWN,
			TrainingGame::RIGHT, TrainingGame::UP,
			TrainingGame::NONE
		};
		static const double random_move_threshold = 0.7;

		double scores[5] = {};
		int px, py, maximum_score = 4;
		std::tie(px, py) = pacman.position();
		scores[4] = EvaluateKernel(px, py, field, ghosts, pacman);

		for (int d = 0; d < 4; d++) {
			scores[d] = SMALLEST;
			if (!Blocked(px + dx[d], py + dy[d], field)) {
				scores[d] = EvaluateKernel(px + dx[d], py + dy[d],
										field, ghosts, pacman);
				if (scores[d] > scores[maximum_score] ||
					(scores[d] == scores[maximum_score] &&
					 zero_to_one.GetRandom() > random_move_threshold)) {
					maximum_score = d;
				}
			}
		}

		return moves[maximum_score];
	}
};

class PacManTrainer {
protected:
	long long timeout_t;
	int total_generations,
		population_per_generation;
	double threshold_mutation,
		   threshold_crossover;
public:
	PacManTrainer(long long to = 2000000, int generations = 100, int population = 100,
				double prob_mutation = 0.60, double prob_crossover = 0.05):
		timeout_t(to),
		total_generations(generations),
		population_per_generation(population),
		threshold_mutation(1. - prob_mutation),
		threshold_crossover(1. - prob_crossover) {}

	// two point crossover
	static inline vector<PacManGene> Crossover(
			PacManGene& p1, PacManGene& p2)
	{
		static const double eps = 1e-9;
		PacManGene c1(p1.get_kernel_size(), p1.get_weights()),
				   c2(p2.get_kernel_size(), p2.get_weights());
		int cross_start = (zero_to_one.GetRandom() + eps) * ((int)c1.size() - 1);
		int cross_end = (zero_to_one.GetRandom() + eps) * ((int)c1.size() - 1);
		if (cross_start > cross_end) swap(cross_start, cross_end);
		for (int i = cross_start; i <= cross_end; i++) {
			swap(c1[i], c2[i]);
		}
		return vector<PacManGene>({c1, c2});
	}

	static inline void Mutate(PacManGene& p) {
		static const double eps = 1e-9;
		int swap_count = (zero_to_one.GetRandom() + eps) * p.size();
		while (swap_count --> 0) {
			int l = (zero_to_one.GetRandom() + eps) * ((int)p.size() - 1),
				r = (zero_to_one.GetRandom() + eps) * ((int)p.size() - 1);
			swap(p[l], p[r]);
		}
	}

	inline vector<PacManGene> ProcessGeneration(
			vector<PacManGene>& current_generations)
	{
		TrainingGame game;
		for (auto& current: current_generations) {
			game.Reset();
			long long t = 0;
			while (!game.is_finished()) {
				game.Process(t, [this, &t, &game, &current](
					const vector<vector<Cell>>& field,
					const vector<Ghost>& ghosts,
					const PacMan& pacman) {
						if (t >= timeout_t) return (int)TrainingGame::ESC;
						return current.DecideMove(field, ghosts, pacman);
					});
				++t;
			}
			current.set_fitness(game.get_score());
			current.set_timed_out(game.is_timed_out());
		}

		// sort: [highest fitness ... lowest fitness].
		sort(current_generations.begin(),
			current_generations.end());

		for (int i = 0; i < 10; i++) {
			printf("Gene #%d: %d [%s[38;5;7m]\n", i, current_generations[i].get_fitness(),
					current_generations[i].is_timed_out() ?
					"[38;5;9mTIMED OUT!" : "[38;5;10mNORMAL");
		}
		puts("");

		vector<PacManGene> next_generations;

		// Preservation of two most successful genes.
		next_generations.emplace_back(current_generations[0]);
		next_generations.emplace_back(current_generations[1]);

		// Crossover phase.
		while (next_generations.size() < population_per_generation) {
			for (int i = 0; i+i < current_generations.size() &&
							next_generations.size() < population_per_generation;
							i++) {
				for (int j = i+1; j+j < current_generations.size() &&
								next_generations.size() < population_per_generation;
								j++) {
					if (zero_to_one.GetRandom() > threshold_crossover) {
						auto children = Crossover(current_generations[i],
												current_generations[j]);
						next_generations.emplace_back(children[0]);
						next_generations.emplace_back(children[1]);
					}
				}
			}
		}

		// Mutation phase.
		for (auto& gene: next_generations) {
			if (zero_to_one.GetRandom() > threshold_mutation)
				Mutate(gene);
		}

		return next_generations;
	}
	
	inline vector<PacManGene> Train(void)
	{
		int current_generation = 0;
		vector<PacManGene> genes;

		while (genes.size() < population_per_generation) {
			genes.emplace_back(PacManGene());
		}

		while (current_generation < total_generations) {
			printf("Generation #%02d:\n", current_generation);
			genes = ProcessGeneration(genes);
			++current_generation;
		}

		return genes;
	}
};
#undef x
#undef y
#endif
