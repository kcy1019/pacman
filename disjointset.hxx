#pragma once
#ifndef __DISJOINTSET__
#define __DISJOINTSET__
#include<vector>
using std::vector;

class DisjointSet {
private:
	vector<int> parent;
	inline int FindRoot(int cur) {
		return parent[cur] = (parent[cur] != cur) ?
							FindRoot(parent[cur]) : cur;
	}
public:
	DisjointSet(int n): parent(n) {
		for (int i = 0; i < n; i++) {
			parent[i] = i;
		}
	}

	inline bool CheckRoot(int x, int y) {
		return FindRoot(x) == FindRoot(y);
	}

	inline void MergeRoot(int from, int to) {
		if (from < parent.size() && from >= 0 &&
			to < parent.size() && to >= 0)
			parent[FindRoot(from)] = parent[FindRoot(to)];
	}
};
#endif
