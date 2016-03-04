#pragma once
#ifndef __SIMPLE_GENETIC__
#define __SIMPLE_GENETIC__
#include<set>
#include<string>
#include<climits>
#include<fstream>
#include"game.nogui.hxx"
#define x first
#define y second
using std::string;
using std::vector;
using std::swap;

static Random<std::uniform_real_distribution<>, double> zero_to_one(0., 1.);
static Random<std::uniform_int_distribution<>, int> rand4(0, 3);
static const int SMALLEST = INT_MIN;

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
            long long current_time,
            const vector<vector<Cell>>& field,
            const vector<Ghost>& ghosts,
            const PacMan& pacman) const {
        double score = 0;
        int px, py, gn = ghosts.size(), gx[gn], gy[gn], ge[gn];
        const int gbase = kernel_size_sq * 3;
        for (int i = 0; i < ghosts.size(); i++) {
            std::tie(gx[i], gy[i]) = ghosts[i].position();
            ge[i] = ghosts[i].edible(current_time) ||
                    field[y][x] == Cell::FRUIT;
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
                        score += (ge[g] ? -1. : 1.) *
                                 weights[gbase + (ky * kernel_size + kx)];
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
            long long current_time,
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
        static const double random_move_threshold = 0.75;

        double scores[5] = {}, g_penalty_mean = 0;
        int px, py, maximum_score = 4;
        std::tie(px, py) = pacman.position();
        scores[4] = EvaluateKernel(px, py, current_time, field, ghosts, pacman);

        for (int i = kernel_size_sq*2; i < weights.size(); i++) {
            g_penalty_mean += -weights[i] / kernel_size_sq;
        }

        for (int d = 0; d < 4; d++) {
            scores[d] = SMALLEST;
            if (!Blocked(px + dx[d], py + dy[d], field)) {
                scores[d] = EvaluateKernel(px + dx[d], py + dy[d],
                                        current_time + 1,
                                        field, ghosts, pacman);
                if (scores[d] > scores[maximum_score] ||
                    (scores[maximum_score] - scores[d] < g_penalty_mean &&
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
    long long max_moves;
    int kernel_size;
    int total_generations,
        population_per_generation;
    double threshold_mutation,
           threshold_crossover;
    string saved_game;

public:
    PacManTrainer(long long to = 2000000,
                  int generations = 100, int population = 100,
                  double prob_mutation = 0.05, double prob_crossover = 0.60,
                  int kn_sz = 3, const string& saved_game = "game1.dat"):
        max_moves(to),
        total_generations(generations),
        population_per_generation(population),
        threshold_mutation(1. - prob_mutation),
        threshold_crossover(1. - prob_crossover),
        kernel_size(kn_sz),
        saved_game(saved_game) {}

    // Two-point crossover
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

    inline std::tuple<PacManGene, vector<PacManGene>> ProcessGeneration(
            vector<PacManGene>& current_generations)
    {
        TrainingGame game(20, 20, 20 / 2, 5, saved_game);
        for (auto& current: current_generations) {
            game.Reset();
            long long t = 0;
            while (!game.is_finished()) {
                game.Process(t, [this, &t, &game, &current](
                    const vector<vector<Cell>>& field,
                    const vector<Ghost>& ghosts,
                    const PacMan& pacman) {
                        if (t >= max_moves) return (int)TrainingGame::ESC;
                        return current.DecideMove(t, field, ghosts, pacman);
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

        // Preserve two most successful genes.
        next_generations.emplace_back(current_generations[0]);
        next_generations.emplace_back(current_generations[1]);

        // Crossover phase.
        while (next_generations.size() < population_per_generation) {
            for (int i = 0; i+i < current_generations.size() &&
                            next_generations.size() < population_per_generation;
                            i++) {
                for (int j = i+1;
                     j+j < current_generations.size() &&
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

        return make_tuple(current_generations[0], next_generations);
    }

    inline vector<PacManGene> Train(void)
    {
        int current_generation = 0;
        vector<PacManGene> genes;

        while (genes.size() < population_per_generation) {
            genes.emplace_back(PacManGene(kernel_size));
        }

        std::set<PacManGene> finest_genes;
        while (current_generation < total_generations) {
            PacManGene finest;
            printf("Generation #%02d:\n", current_generation);
            std::tie(finest, genes) = ProcessGeneration(genes);
            if (finest_genes.size() > 10) {
                finest_genes.erase(finest_genes.begin());
            }
            ++current_generation;
        }

        TrainingGame game(20, 20, 20 / 2, 5, saved_game);
        for (auto& gene: genes) {
            game.Reset();
            long long t = 0;
            while (!game.is_finished()) {
                game.Process(t, [this, &t, &game, &gene](
                    const vector<vector<Cell>>& field,
                    const vector<Ghost>& ghosts,
                    const PacMan& pacman) {
                        if (t >= max_moves) return (int)TrainingGame::ESC;
                        return gene.DecideMove(t, field, ghosts, pacman);
                    });
                ++t;
            }
            gene.set_fitness(game.get_score());
            gene.set_timed_out(game.is_timed_out());
        }

        std::sort(genes.begin(), genes.end());
        return genes;
    }
};
#undef x
#undef y
#endif
