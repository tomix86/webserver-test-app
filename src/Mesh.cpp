#include "Mesh.hpp"

#include <cmath>
#include <iostream>

//externs
size_t STEPS;
size_t MESH_SIZE;

float Mesh::ENVIRONMENT_TEMP = 50.f;
float Mesh::INITIAL_TEMP = -40.f;
Mesh::ContainerType Mesh::temperature;

bool VALIDATE_RESULTS = false;

void setValidateResults(bool value) {
	VALIDATE_RESULTS = value;
}

float* allocMeshLinear(size_t& pitch, size_t size) {
	pitch = size * sizeof(float);
	float* output;

	try {
		output = new float[size * size];

		// Set temperature for guardian fields
		for (int i = 0; i < size; ++i) {
			for (int j = 0; j < size; ++j) {
				if (i == 0 || i == size - 1 || j == 0 || j == size - 1) {
					*getElem(output, pitch, i, j) = Mesh::ENVIRONMENT_TEMP;
				} else {
					*getElem(output, pitch, i, j) = Mesh::INITIAL_TEMP;
				}
			}
		}

		return output;
	} catch (std::exception ex) {
		std::cout << ex.what() << '\n';
		exit(-1);
	}
}

bool validateResults(float* input, size_t pitch) {
	if (VALIDATE_RESULTS) {
		for (int i = 0; i < MESH_SIZE; ++i) {
			for (int j = 0; j < MESH_SIZE; ++j) {
				const float threshold = .001f;
				const auto value = *getElem(input, pitch, i + 1, j + 1);
				if (fabs(Mesh::temperature[i][j] - value) > threshold) {
					std::cout << "Results are different for optimized version!\n"
							  << "Mismatch for [" << i << "][" << j << "]: " << Mesh::temperature[i][j] << " vs " << value << '\n';
					return false;
				}
			}
		}
	}

	return true;
}