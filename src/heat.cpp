#include <iostream>

#include "Mesh.hpp"
#include "SimpleTimer.hpp"

void initialize_heat_compute() {
	//TODO: parameterize
	setValidateResults(false);
	STEPS = 1;
	MESH_SIZE = 100;
	std::cout << "Single Allocation size will be: " << sizeof(float) * MESH_SIZE * MESH_SIZE / (1024 * 1024) << "MiB" << std::endl;
}

void basic_heat_compute() {
	Mesh m;
	Mesh::ContainerType nodes;

	try {
		m.resize(MESH_SIZE);
		nodes = Mesh::temperature;
	} catch (std::exception ex) {
		std::cout << ex.what() << std::endl;
		exit(-1);
	}

	SimpleTimer t{ "Basic implementation" };
	for (size_t step = 0; step < STEPS; ++step) {
		//OpenMP 2.0 does not have collapse(2)...
#pragma omp parallel for shared(nodes)
		for (long long int t = 0; t < MESH_SIZE * MESH_SIZE; ++t) {
			size_t x = t / MESH_SIZE;
			size_t y = t % MESH_SIZE;
			float t_l = Mesh::getTemperature(x - 1, y); // temperature of left neighbour [K]
			float t_r = Mesh::getTemperature(x + 1, y); // temperature of right neighbour [K]
			float t_t = Mesh::getTemperature(x, y - 1); // temperature of top neighbour [K]
			float t_b = Mesh::getTemperature(x, y + 1); // temperature of bottom neighbour [K]

			const float k = 1.f; // thermal conductivity [W/m*K]
			const float d_x = 1.f; // delta x [m]
			float q = 0.f; // heat flux [W/m^3]
			float newTemperature = (t_l + t_r + t_b + t_t + q * d_x * d_x / k) / 4;

			nodes[y][x] = newTemperature;
		}

		Mesh::temperature.swap(nodes);
	}
}
