// Ricardas Navickas 2020
#ifndef NOISE_H
#define NOISE_H

#include <map>
#include "core/core.h"

// 1-dimensional Perlin noise generator
class Noise1d {
public:
	Noise1d(float cs);
	~Noise1d();

	// Returns noise value at chosen position
	float get_value(float pos);
	float get_value(float pos, int n); // sum n octaves

	// Distance between adjacent gradient vectors
	float cell_size;

private:
	// 3d map of gradient vectors
	std::map<int, float> gradients;

	bool gradient_exists(int x);
	void generate_gradient(int x);
};

// 3-dimensional Perlin noise generator
class Noise3d {
public:
	Noise3d(float cs);
	~Noise3d();

	// Returns noise value at chosen position
	float get_value(glm::vec3 pos);
	float get_value(glm::vec3 pos, int n); // sum n octaves

	// Distance between adjacent gradient vectors
	float cell_size;

private:
	// 3d map of gradient vectors
	std::map<int, std::map<int, std::map<int, glm::vec3>>> gradients;

	bool gradient_exists(int x, int y, int z);
	void generate_gradient(int x, int y, int z);
};

#endif
