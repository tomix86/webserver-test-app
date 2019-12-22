#include <iostream>

#include "Mesh.hpp"
#include "SimpleTimer.hpp"
#include "spdlog/spdlog.h"

void initialize_heat_compute() {
	//TODO: parameterize
	setValidateResults(false);
	STEPS = 1;
	MESH_SIZE = 100;
	spdlog::info("Single Allocation size will be: {} MiB", sizeof(float) * MESH_SIZE * MESH_SIZE / (1024 * 1024));
}

void basic_heat_compute() {
	Mesh m;
	Mesh::ContainerType nodes;

	try {
		m.resize(MESH_SIZE);
		nodes = Mesh::temperature;
	} catch (const std::exception& ex) {
		spdlog::error(ex.what());
		exit(-1);
	}

	SimpleTimer t{ "Basic implementation", true };
	for (size_t step = 0; step < STEPS; ++step) {
		//OpenMP 2.0 does not have collapse(2)...
#pragma omp parallel for shared(nodes)
		for (long long int t = 0; t < MESH_SIZE * MESH_SIZE; ++t) {
			const auto x = t / MESH_SIZE;
			const auto y = t % MESH_SIZE;
			const auto t_l = Mesh::getTemperature(x - 1, y); // temperature of left neighbour [K]
			const auto t_r = Mesh::getTemperature(x + 1, y); // temperature of right neighbour [K]
			const auto t_t = Mesh::getTemperature(x, y - 1); // temperature of top neighbour [K]
			const auto t_b = Mesh::getTemperature(x, y + 1); // temperature of bottom neighbour [K]

			const auto k = 1.f; // thermal conductivity [W/m*K]
			const auto d_x = 1.f; // delta x [m]
			const auto q = 0.f; // heat flux [W/m^3]
			const auto newTemperature = (t_l + t_r + t_b + t_t + q * d_x * d_x / k) / 4;

			nodes[y][x] = newTemperature;
		}

		Mesh::temperature.swap(nodes);
	}
}
