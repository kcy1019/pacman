#pragma once
#ifndef __MAZE_GENERATOR__
#define __MAZE_GENERATOR__
#define x first
#define y second
#include<queue>
#include<vector>
#include<random>
#include<algorithm>
#include"random.hxx"
#include"disjointset.hxx"
using std::pair;
using std::queue;
using std::vector;

template<typename CellType>
class MazeFactory {
private:
	typedef std::uniform_real_distribution<> RealDistribution;
	typedef std::uniform_int_distribution<> IntDistribution;
	inline bool CheckBoundary(int x, int y, int width, int height) {
		return x >= 0 && y >= 0 && x < width && y < height;
	}
	inline int GetIndex(int x, int y, int width) {
		return x * width + y;
	}
public:
	MazeFactory() {}
	vector<vector<CellType>> GenerateMaze(int width, int height, double cycle_acceptance = 0.35)
	{
		static const int dx[] = {-1, 0, 1, 0},
						 dy[] = {0, 1, 0, -1};
		static const double _threshold = 0.5;
		int n_cell = width * height;
		DisjointSet cell_set(n_cell);
		Random<IntDistribution, int> hgen(0, height-1), wgen(0, width-1);
		Random<RealDistribution, double> real(0., 1.);
		vector<vector<CellType>> field(height, vector<CellType>(width, CellType::WALL));

		for (int i = 0; i < height; i+=2) {
			for (int j = 0; j < width; j+=2)
				field[i][j] = CellType::EMPTY;
		}

		for (int k = 0; k < 2; k++) {
			double threshold = k ? 0. : _threshold;
			for (int i = 0; i < height; i+=2) {
				for (int j = 0; j+2 < width; j+=2) {
					if (field[i][j+1] == CellType::WALL &&
							(!cell_set.CheckRoot(GetIndex(j, i, width), GetIndex(j+2, i, width)) ||
							 real.GetRandom() > cycle_acceptance) &&
							real.GetRandom() > threshold) {
						field[i][j+1] = CellType::EMPTY;
						cell_set.MergeRoot(GetIndex(j, i, width), GetIndex(j+2, i, width));
					}
				}
			}

			for (int i = 0; i+2 < height; i+=2) {
				for (int j = 0; j < width; j+=2) {
					if (field[i+1][j] == CellType::WALL &&
							(!cell_set.CheckRoot(GetIndex(j, i, width), GetIndex(j, i+2, width)) ||
							 real.GetRandom() > cycle_acceptance) &&
							real.GetRandom() > threshold) {
						field[i+1][j] = CellType::EMPTY;
						cell_set.MergeRoot(GetIndex(j, i, width), GetIndex(j, i+2, width));
					}
				}
			}
		}

		return field;
	}
};
#undef x
#undef y
#endif
