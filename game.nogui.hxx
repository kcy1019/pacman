#pragma once
#ifndef __GAME_NOGUI__
#define __GAME_NOGUI__
#include<cassert>
#include<vector>
#include<string>
#include<algorithm>
#include<random>
#include"maze.hxx"
#include"random.hxx"
#include"ghost.hxx"
#include"pacman.hxx"
using std::vector;
using std::string;
#define x first
#define y second

static Random<std::uniform_real_distribution<>, double> real1(0., 1.);
static Random<std::uniform_int_distribution<>, int> mod4(0, 3);

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

// Game without GUI(+ncurses) and messages.
// With this, one can play multiple games on the same board.
// TODO: Redsign - use inheritance; inherit class Game, then override.
static const int GHOST_RESPAWN_DELAY = 500;
static const int GHOST_EDIBLE_DURATION = 500;
#endif

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

    TrainingGame(int height = 20, int width = 20, int sight = 10, int nghosts = 5,
                 const string& saved_game = "NONE"):
        height(height), width(width), score(0),
        pacman(), sight(sight), nghosts(nghosts),
        _nbiscuits(0), _nfruits(0), ghosts()
    {
        bool null_filename = saved_game == "NONE";
        bool load_success = ImportGame(saved_game);
        if (null_filename || !load_success) {
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
                            real1.GetRandom() > fruit_dist) {
                        _field[i][j] = Cell::FRUIT;
                        ++_nfruits;
                        --_nbiscuits;
                    }
                }
            }

            if (!null_filename) {
                ExportGame(saved_game);
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
        for (int i = (height + 1) / 2; i >= 0 && !escape_loop; i--) {
            for (int j = (width + 1) / 2; j >= 0 && !escape_loop; j--) {
                if (field[i][j] != Cell::WALL) {
                    int sz_ghost = ghosts.size();
                    ghosts.emplace_back(Ghost(j, i, colors[sz_ghost % 5]));
                    if (ghosts.size() == nghosts)
                        escape_loop = true;
                }
            }
        }
    }

    inline bool ImportGame(const string& filename) {
        std::fstream fs(filename, std::ios::in);
        if (!fs.is_open()) {
            return false;
        }

        fs >> width >> height;
        fs >> _nbiscuits >> _nfruits;
        fs >> sight >> nghosts;
        _field.resize(height, vector<Cell>(width, Cell::WALL));

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int _type;
                fs >> _type;
                _field[i][j].type = _type;
            }
        }

        fs.close();
        return true;
    }

    inline void ExportGame(const string& filename) {
        std::fstream fs(filename, std::ios::out);
        fs << width << " " << height << std::endl;
        fs << _nbiscuits << " " << _nfruits << std::endl;
        fs << sight << " " << nghosts << std::endl;

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int _type = _field[i][j].type;
                fs << _type << " ";
            }
            fs << std::endl;
        }

        fs.close();
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
            y = pacman.position().y,
            gpx[ghosts.size()],
            gpy[ghosts.size()],
            ged[ghosts.size()],
            gdd[ghosts.size()],
            ptr;

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

        ptr = 0;
        for (auto& ghost: ghosts) {
            int gx = ghost.position().x,
                gy = ghost.position().y,
                ge = ghost.edible(current_time),
                gd = ghost.dead(current_time),
                cur_distance = Distance(gx, gy, x, y);
            std::tie(gpx[ptr], gpy[ptr], ged[ptr], gdd[ptr]) =
                std::tie(gx, gy, ge, gd);
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
                    int k = mod4.GetRandom();
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
                ghost.set_edible(current_time + GHOST_EDIBLE_DURATION);
            }
        }

        int nx = x, ny = y;
        switch(stream(field, ghosts, pacman)) {
            case LEFT:
                if (!Blocked(x-1, y)) {
                    pacman.move(-1, 0);
                    nx -= 1;
                }
                break;
            case RIGHT:
                if (!Blocked(x+1, y)) {
                    pacman.move(1, 0);
                    nx += 1;
                }
                break;
            case UP:
                if (!Blocked(x, y-1)) {
                    pacman.move(0, -1);
                    ny -= 1;
                }
                break;
            case DOWN:
                if (!Blocked(x, y+1)) {
                    pacman.move(0, 1);
                    ny += 1;
                }
                break;
            case ESC:
                // In case of AI controller, this can be 'timeout'.
                timeout = true;
                EndGame();
        }

        for (int i = 0; i < ghosts.size(); i++) {
            if (ghosts[i].position() == std::make_pair(x, y) &&
                pacman.position() == std::make_pair(gpx[i], gpy[i])) {
                if (ged[i]) {
                    ghosts[i].set_dead(current_time + GHOST_RESPAWN_DELAY);
                } else if (!gdd[i]) {
                    if (pacman.lives_left()) {
                        pacman.set_dead();
                    } else {
                        EndGame();
                    }
                }
            }
        }

    }

    inline void EndGame(void) {
        finished = true;
    }

};

#undef x
#undef y
#endif
