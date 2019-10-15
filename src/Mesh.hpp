#pragma once
#include <vector>

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

extern size_t STEPS;
extern size_t MESH_SIZE;

class Mesh {
public:
	typedef std::vector<std::vector<float>> ContainerType;

	static float ENVIRONMENT_TEMP;
	static float INITIAL_TEMP;
	static ContainerType temperature;

	void resize(size_t size) { temperature = ContainerType(size, std::vector<float>(size, INITIAL_TEMP)); }

	static float getTemperature(int x, int y) {
		if (x < 0 || y < 0 || x >= temperature[0].size() || y >= temperature.size()) { return ENVIRONMENT_TEMP; }
		return temperature[y][x];
	}
};

template <typename T> __host__ __device__ inline T* getElem(T* BaseAddress, size_t pitch, unsigned Row, unsigned Column) {
	return reinterpret_cast<T*>(reinterpret_cast<char*>(BaseAddress) + Row * pitch) + Column;
}

float* allocMeshLinear(size_t& pitch, size_t size);
bool validateResults(float* input, size_t pitch);
void setValidateResults(bool value);
