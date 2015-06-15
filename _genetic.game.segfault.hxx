#pragma once
#ifndef __GENETIC_GAME__
#define __GENETIC_GAME__
#include<vector>
#include<random>
#include<string>
#include<cassert>
#include<algorithm>
#include"maze.hxx"
#include"ghost.hxx"
#include"random.hxx"
#include"pacman.hxx"
#include"game.hxx"
#define x first
#define y second 
using std::string;
using std::vector;
using std::swap;

class DefaultGene {
protected:
	typedef std::uniform_real_distribution<> RealDistribution;
	typedef std::uniform_int_distribution<> IntDistribution;
	Random<RealDistribution, double> zero_to_one;
	Random<RealDistribution, double> real;
	vector<double> gene;
	int fitness;
public:
	DefaultGene(const vector<double>& v): gene(v.begin(), v.end()),
								   real(0., 128.), fitness(-1),
								   zero_to_one(0., 1.) {}
	DefaultGene(const vector<double>&& v): gene(v.begin(), v.end()),
								   real(0., 128.), fitness(-1),
								   zero_to_one(0., 1.) {}

	DefaultGene(const DefaultGene& v): gene(v.begin(), v.end()),
						   real(0., 128.), fitness(v.fitness),
						   zero_to_one(0., 1.) {}
	DefaultGene(const DefaultGene&& v): gene(v.begin(), v.end()),
						   real(0., 128.), fitness(v.fitness),
						   zero_to_one(0., 1.) {}

	void operator = (const DefaultGene& r) {
		fitness = r.fitness;
		gene = r.gene;
	}

	DefaultGene(int n): gene(n),
				real(0., 128.),
				zero_to_one(0., 1.),
				fitness(-1)
	{
		for (int i = 0; i < n; i++)
			gene[i] = real.GetRandom();
	}
	// Mutation Operator
	inline DefaultGene operator! (void) {
		vector<double> mutant(gene.begin(), gene.end());
		int swap_count = zero_to_one.GetRandom() * mutant.size();
		while (swap_count --> 0) {
			int l = real.GetRandom() * ((int)mutant.size() - 1),
				r = real.GetRandom() * ((int)mutant.size() - 1);
			swap(mutant[l], mutant[r]);
		}
		return DefaultGene(mutant);
	}
	// Crossover Operator
	// two point crossover
	inline vector<DefaultGene> operator^ (const DefaultGene& r) {
		vector<double> offspring1(gene.begin(), gene.end()),
						offspring2(r.begin(), r.end());
		int cross_start = zero_to_one.GetRandom() * ((int)offspring1.size() - 1);
		int cross_end = zero_to_one.GetRandom() * ((int)offspring1.size() - 1);
		if (cross_start > cross_end) swap(cross_start, cross_end);
		for (int i = cross_start; i <= cross_end; i++) {
			swap(offspring1[i], offspring2[i]);
		}
		return vector<DefaultGene>({DefaultGene(offspring1), DefaultGene(offspring2)});
	}
	// Access Operator
	inline double& operator[] (const int i) {
		return gene[i];
	}
	inline vector<double>::const_iterator begin(void) const {
		return gene.cbegin();
	}
	inline vector<double>::const_iterator end(void) const {
		return gene.cend();
	}
	// Maximum 2000 floating point weights per gene.
	inline string ToString(void) const {
		char output[32768] = {};
		int ptr = 0;
		for (int i = 0; i < gene.size(); i++) {
			ptr += sprintf(output + ptr, "%.14f", gene[i]);
		}
		return string(output);
	}
	inline void LoadData(const vector<double>& data) {
		gene = data;
	}
	inline int get_fitness(void) const {
		return fitness;
	}
	inline void set_fitness(int _fitness) {
		fitness = _fitness;
	}
};

// Simple Square Kernel Evaluator
class PacManGene: public DefaultGene {
protected:
	int kernel_size, kernel_size_sq;
public:
	PacManGene(int kernel_size = 5):
		DefaultGene(4 * kernel_size * kernel_size),
		kernel_size_sq(kernel_size * kernel_size) {
			assert(kernel_size % 2 == 1 ||
					"PacManGene: Kernel Size must be odd." == 0);
			for (int i = 0; i < kernel_size_sq; i++) {
				gene[kernel_size_sq * 3 + i] *= -1;
			}
		}

	PacManGene(DefaultGene gene_, int kernel_size = 5):
		DefaultGene(gene_),
		kernel_size(kernel_size),
		kernel_size_sq(kernel_size * kernel_size) {}

	void operator = (const PacManGene& r) {
		gene = r.gene;
		kernel_size = r.kernel_size;
		kernel_size_sq = r.kernel_size_sq;
		fitness = r.fitness;
	}

	inline double EvaluateKernel(int x, int y,
			const vector<vector<Cell>>& field,
			const vector<Ghost>& ghosts,
			const PacMan& pacman) const {
		double score = 0;
		for (int ky = 0; ky < kernel_size; ky++) {
			int h = -kernel_size/2 + ky + y;
			if (h < 0 || h >= field.size()) continue;
			for (int kx = 0; kx < kernel_size; kx++) {
				int w = -kernel_size/2 + kx + x;
				if (w < 0 || w >= field[0].size()) continue;
				bool seen = false;
				for (const auto& ghost: ghosts) {
					int gx, gy;
					std::tie(gx, gy) = ghost.position();
					if (gx == w && gy == h) {
						seen = true;
						score += gene[(kernel_size_sq * 3) + (ky * kernel_size + kx)];
						break;
					}
				}

				if (seen) continue;

				switch(field[y][x].type) {
					case Cell::EMPTY:
						score += gene[(ky * kernel_size + kx)];
						break;
					case Cell::BISCUIT:
						score += gene[kernel_size_sq + (ky * kernel_size + kx)];
						break;
					case Cell::FRUIT:
						score += gene[(kernel_size_sq * 2) + (ky * kernel_size + kx)];
						break;
					case Cell::WALL:
						break;
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

	int Behave(const vector<vector<Cell>>& field,
			const vector<Ghost>& ghosts,
			const PacMan& pacman) const
	{
		static const int dx[] = {-1, 0, 1, 0},
						 dy[] = {0, 1, 0, -1},
						 signal[] = {Game::LEFT, Game::DOWN,
							 Game::RIGHT, Game::UP, Game::NONE};
		int x, y, max_sig = 4;
		double eval_res[5] = {};
		std::tie(x, y) = pacman.position();
		eval_res[4] = EvaluateKernel(x, y, field, ghosts, pacman);
		for (int k = 0; k < 4; k++) {
			eval_res[k] = -1e38;
			int nx = x + dx[k],
				ny = y + dy[k];
			if (!Blocked(nx, ny, field)) {
				eval_res[k] = EvaluateKernel(nx, ny, field, ghosts, pacman);
				if (eval_res[k] > eval_res[max_sig])
					max_sig = k;
			}
		}
		return signal[max_sig];
	}

	// Mutation Operator
	inline PacManGene operator! (void) {
		return PacManGene(!DefaultGene(gene), kernel_size);
	}
	// Crossover Operator
	// two point crossover
	inline vector<PacManGene> operator^ (const PacManGene& r) {
		vector<DefaultGene>&& temp_gens = 
			DefaultGene(gene) ^ DefaultGene(r.gene);
		return vector<PacManGene>({
					PacManGene(temp_gens[0]),
					PacManGene(temp_gens[1])
				});
	}
};

template<typename GeneType = PacManGene>
class TrainingGame: public Game {
protected:
	typedef std::uniform_real_distribution<> RealDistribution;
	typedef std::uniform_int_distribution<> IntDistribution;
	bool finished;
	GeneType genome;
public:
	TrainingGame(const TrainingGame&& g):
		Game(g.width, g.height, g.sight, g.nghosts),
		genome(), finished(false) {}

	TrainingGame(const TrainingGame& g):
		Game(g.width, g.height, g.sight, g.nghosts),
		genome(), finished(false) {}

	TrainingGame(int width = 20, int height = 20,
				int sight = 10, int nghosts = 5):
		Game(width, height, sight, nghosts),
		genome(), finished(false) {}

	TrainingGame(const GeneType& genome,
			int width = 20, int height = 20,
			int sight = 10, int nghosts = 5):
		Game(width, height, sight, nghosts),
		genome(genome), finished(false) {}

	void operator=(const TrainingGame<PacManGene>& r) {
		width = r.width;
		height = r.height;
		sight = r.sight;
		nghosts = r.nghosts;
		genome = r.genome;
		finished = r.finished;
		field = r.field;
		nbiscuits = r.nbiscuits;
		nfruits = r.nfruits;
		ghosts = r.ghosts;
		pacman = r.pacman;
	}

	inline bool operator < (const TrainingGame<GeneType>& game) const {
		int le = genome.get_fitness();
		int ri = game.get_const_genome().get_fitness();
		if (le == -1 && ri == -1) return false;
		if (ri == -1) return false;
		if (le == -1) return true;
		return le > ri;
	}

	// Overloading
	inline void DrawLegends(void) {}
	inline void Draw(long long) {}

	inline GeneType& get_genome(void) {
		return genome;
	}

	inline const GeneType& get_const_genome(void) const {
		return (const GeneType&)genome;
	}

	template<typename Function>
	inline void Process(long long current_time, const Function& input_source)
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
		switch(input_source(field, ghosts, pacman)) {
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
				EndGame();
		}
	}

	inline void EndGame(void) {
		genome.set_fitness(score);
		finished = true;
	}

	inline bool IsFinished(void) {
		return finished;
	}

};

template<typename GeneType = PacManGene,
		typename GameType = TrainingGame<PacManGene>>
class GeneticAlgorithm {
protected:
	typedef std::uniform_real_distribution<> RealDistribution;
	typedef std::uniform_int_distribution<> IntDistribution;
	Random<RealDistribution, double> zero_to_one;

	int cur_generation,
		tot_generations,
		population_per_generation;
	double threshold_mutation,
		   threshold_crossover;

public:
	GeneticAlgorithm(int generations = 100, int population = 100,
					double prob_mutation = 0.60, double prob_crossover = 0.05):
		cur_generation(0),
		tot_generations(generations),
		population_per_generation(population),
		threshold_mutation(1. - prob_mutation),
		threshold_crossover(1. - prob_crossover),
		zero_to_one(0., 1.) {}

	inline void ProcessOneGame(GameType& game)
	{
		long long t = 0;
		while (!game.IsFinished()) {
			game.Process(++t, [&game](
						const vector<vector<Cell>>& field,
						const vector<Ghost>& ghosts,
						const PacMan& pacman) {
						return game.get_genome().Behave(
							field, ghosts, pacman);
					});
		}
	}

	inline void ProcessGeneration(vector<GameType>& generation)
	{
		for (auto& game: generation) {
			ProcessOneGame(game);
		}
	}

	inline vector<GameType> Train(void)
	{
		vector<GameType> current_games;
		while (current_games.size() < population_per_generation) {
			current_games.emplace_back(GameType());
		}
		for (; cur_generation < tot_generations; ++cur_generation) {
			ProcessGeneration(current_games);
			// Decreasing Order - by Fitness.
			sort(current_games.begin(), current_games.end());
			// Preserve top 2, and perform crossover on top 50%.
			vector<GameType> temp_games, next_games;
			temp_games.emplace_back(current_games[0]);
			temp_games.emplace_back(current_games[1]);
			while (temp_games.size() < population_per_generation) {
				for (int i = 0; i+i < current_games.size() && 
								temp_games.size() < population_per_generation;
								i++)
					for (int j = i+1; j+j <= current_games.size() &&
								temp_games.size() < population_per_generation;
								j++)
						if (zero_to_one.GetRandom() > threshold_crossover) {
							auto&& v = current_games[i].get_genome() ^ current_games[j].get_genome();
							temp_games.emplace_back(GameType(v[0]));
							temp_games.emplace_back(GameType(v[1]));
						}
			}
			// Perform mutation.
			for (auto& game: temp_games) {
				if (zero_to_one.GetRandom() > threshold_mutation)
					next_games.emplace_back(GameType(!game.get_genome()));
				else
					next_games.emplace_back(game);
			}

			printf("Generation #%02d:\n", cur_generation);
			for (int i = 0; i < 10; i++) {
				printf("#%02d: %6d\n", current_games[i].get_genome().get_fitness());
			}

			current_games = next_games;
		}
		return current_games;
	}
};

#undef x
#undef y
#endif
