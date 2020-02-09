#pragma once
#include <vector>
#include <memory>

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

extern size_t STEPS;
extern long long int MESH_SIZE;

class Mesh {
public:
	typedef std::vector<std::vector<float>> ContainerType;

	static float ENVIRONMENT_TEMP;
	static float INITIAL_TEMP;
	static ContainerType temperature;

	void resize(size_t size) { temperature = ContainerType(size, std::vector<float>(size, INITIAL_TEMP)); }

	static float getTemperature(long long int x, long long int y) {
		if (x < 0 || y < 0 || x >= static_cast<long long int>(temperature[0].size()) || y >= static_cast<long long int>(temperature.size())) {
			return ENVIRONMENT_TEMP;
		}
		return temperature[y][x];
	}
};

template <typename T> __host__ __device__ inline T* getElem(T* BaseAddress, size_t pitch, unsigned Row, unsigned Column) {
	return reinterpret_cast<T*>(reinterpret_cast<char*>(BaseAddress) + Row * pitch) + Column;
}

std::unique_ptr<float[]> allocMeshLinear(size_t& pitch, size_t size);
bool validateResults(float* input, size_t pitch);
void setValidateResults(bool value);
