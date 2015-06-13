#pragma once
#ifndef __RNG__
#define __RNG__
#include<random>

template<typename DistType, typename ReturnType>
class Random {
private:
	std::mt19937 generator;
	std::random_device random_device;
	DistType dist;

	void operator = (Random const&) = delete;
	Random(Random const&) = delete;
public:
	Random(ReturnType from, ReturnType to): random_device() {
		generator = std::mt19937(random_device());
		dist = DistType(from, to);
	}

	ReturnType GetRandom(void) {
		return dist(generator);
	}
};
#endif
