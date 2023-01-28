// Ricardas Navickas 2020
#include "noise.h"
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <ctime>

static int round(float x) {
	return std::floor(x + 0.5f);
}

// ==== ==== 1D NOISE ==== ====
Noise1d::Noise1d(float cs) {
	srand(time(NULL));
	cell_size = cs;
}

Noise1d::~Noise1d() {}

float Noise1d::get_value(float pos) {	
	// Normalized position
	float np = pos / cell_size;

	// Coords of adjacent gradient vectors
	int gc[2] = { round(std::floor(np)), round(std::floor(np + 1)) };

	// Adjacent gradients
	float g[2];
	if (!gradient_exists(gc[0])) generate_gradient(gc[0]);
	if (!gradient_exists(gc[1])) generate_gradient(gc[1]);
	g[0] = gradients[gc[0]];
	g[1] = gradients[gc[1]];

	// Distance to gradient vectors
	float dist[2];
	dist[0] = np - gc[0];
	dist[1] = np - gc[1];

	// Dot products
	float dp[2];
	dp[0] = dist[0] * g[0];
	dp[1] = dist[1] * g[1];

	// Interpolation
	float w[2]; // weights
	w[0] = dist[0] * dist[0] * (3.0f - 2.0f * dist[0]);
	w[1] = 1.0f - w[0];

	//float interp = dp[0] * std::abs(dist[1]) + dp[1] * std::abs(dist[0]);
	float interp = dp[0] * w[1] + dp[1] * w[0];

	return interp;
}

float Noise1d::get_value(float pos, int n) {
	float val = 0.0f;
	float old_cell_size = cell_size;

	for (int i = 0; i < n; i++) {
		val += get_value(pos) / std::pow(2, i);
		cell_size /= 2;
	}

	cell_size = old_cell_size;
	return val;
}

bool Noise1d::gradient_exists(int x) {
	return gradients.count(x) != 0;
}

void Noise1d::generate_gradient(int x) {
	// random float from -1.0 to 1.0
	float new_gradient = float((rand() % 201) - 100) / 100;
	gradients[x] = new_gradient;
}

// ==== ==== 3D NOISE ==== ====
Noise3d::Noise3d(float cs) {
	srand(time(NULL));
	cell_size = cs;
}

Noise3d::~Noise3d() {}

float Noise3d::get_value(glm::vec3 pos) {
	// Convert position to normalized coords
	glm::vec3 np = pos / cell_size;

	// Coords of adjacent gradient vectors
	int gc[8][3] = {
		round(std::floor(np.x)),     round(std::floor(np.y)),     round(std::floor(np.z)),
		round(std::floor(np.x)),     round(std::floor(np.y)),     round(std::floor(np.z) + 1),
		round(std::floor(np.x)),     round(std::floor(np.y) + 1), round(std::floor(np.z)),
		round(std::floor(np.x)),     round(std::floor(np.y) + 1), round(std::floor(np.z) + 1),
		round(std::floor(np.x) + 1), round(std::floor(np.y)),     round(std::floor(np.z)),
		round(std::floor(np.x) + 1), round(std::floor(np.y)),     round(std::floor(np.z) + 1),
		round(std::floor(np.x) + 1), round(std::floor(np.y) + 1), round(std::floor(np.z)),
		round(std::floor(np.x) + 1), round(std::floor(np.y) + 1), round(std::floor(np.z) + 1),
	};

	// Adjacent gradient vectors
	glm::vec3 g[8];
	for (int i = 0; i < 8; i++) {
		int x = gc[i][0];
		int y = gc[i][1];
		int z = gc[i][2];

		if (!gradient_exists(x, y, z)) {
			generate_gradient(x, y, z);
		}

		g[i] = gradients[x][y][z];
	}

	// Distance vectors
	glm::vec3 dist[8];
	for (int i = 0; i < 8; i++) {
		dist[i] = np - glm::vec3(gc[i][0], gc[i][1], gc[i][2]);
	}

	// Dot products
	float dp[8];
	for (int i = 0; i < 8; i++) {
		dp[i] = glm::dot(dist[i], g[i]);
	}

	// Trilinear interpolation
	float interp = 0.0f;
	float w[8]; // weights for each adjacent vertex
	for (int i = 0; i < 8; i++) {
		float dx = std::abs(np.x - gc[i][0]);
		float dy = std::abs(np.y - gc[i][1]);
		float dz = std::abs(np.z - gc[i][2]);
		w[i] = (1.0f - dx) * (1.0f - dy) * (1.0f - dz);

		interp += w[i] * dp[i];
	}

	return interp;
}

float Noise3d::get_value(glm::vec3 pos, int n) {
	float val = 0.0f;
	float old_cell_size = cell_size;

	for (int i = 0; i < n; i++) {
		val += get_value(pos) / std::pow(2, i);
		cell_size /= 2;
	}

	cell_size = old_cell_size;
	return val;
}

bool Noise3d::gradient_exists(int x, int y, int z) {
	return gradients[x][y].count(z) != 0;
}

void Noise3d::generate_gradient(int x, int y, int z) {
	glm::vec3 new_gradient;

	// random float from -1.0 to 1.0
	new_gradient.x = float((rand() % 201) - 100) / 100;
	new_gradient.y = float((rand() % 201) - 100) / 100;
	new_gradient.z = float((rand() % 201) - 100) / 100;

	gradients[x][y][z] = new_gradient;
}

