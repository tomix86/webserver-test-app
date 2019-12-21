#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "Mesh.hpp"
#include "SimpleTimer.hpp"

//TODO: throw exception outside and use spdlog there, otherwise problems wit compilation

static std::string to_string(cudaError_t error) {
	char buf[256];
	snprintf(buf, 256, "%d", error);
	return buf;
}

class CudaError : public std::runtime_error {
public:
	CudaError(std::string source, cudaError_t errorCode) :
		std::runtime_error(source + ": code" + to_string(errorCode) + ": " + cudaGetErrorString(errorCode)) {}
};

#define checkCudaErrors(val) checkError((val), #val, __FILE__, __LINE__)
void checkError(cudaError_t result, const char* calledFunc, const char* file, int line) {
	if (result) {
		std::ostringstream ss;
		ss << file << ": " << line << " {" << calledFunc << '}';

		throw CudaError(ss.str(), result);
	}
}

__global__ void meshUpdateKernel(float* mesh_in, float* mesh_out, size_t pitch, unsigned size) {
	const int x = blockIdx.x * blockDim.x + threadIdx.x;
	const int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x > 0 && x < size - 1 && y > 0 && y < size - 1) {
		const float t_left = *getElem(mesh_in, pitch, y, x - 1);
		const float t_right = *getElem(mesh_in, pitch, y, x + 1);
		const float t_top = *getElem(mesh_in, pitch, y - 1, x);
		const float t_bottom = *getElem(mesh_in, pitch, y + 1, x);

		const float newTemperature = (t_left + t_right + t_top + t_bottom) / 4;

		*getElem(mesh_out, pitch, y, x) = newTemperature;
	}
}

std::vector<std::vector<float>> cuda_heat_compute(int blockDimX, int blockDimY, int meshSize, int steps) {
	meshSize += 2; // add edge rows/cols resembling environment temperature

	size_t pitch;
	float* temperature = allocMeshLinear(pitch, meshSize);
	size_t d_pitch;
	float *d_temperature_in, *d_temperature_out;

	checkCudaErrors(cudaMallocPitch(&d_temperature_in, &d_pitch, meshSize * sizeof(float), meshSize));
	checkCudaErrors(cudaMallocPitch(&d_temperature_out, &d_pitch, meshSize * sizeof(float), meshSize));

	{
		SimpleTimer t("CUDA computations");
		dim3 blockSize(blockDimX, blockDimY);
		unsigned computedGridDimX = (meshSize + blockSize.x - 1) / blockSize.x;
		unsigned computedGridDimY = (meshSize + blockSize.y - 1) / blockSize.y;
		dim3 gridSize(computedGridDimX, computedGridDimY);

		checkCudaErrors(
			cudaMemcpy2D(d_temperature_in, d_pitch, temperature, pitch, meshSize * sizeof(float), meshSize, cudaMemcpyHostToDevice));
		checkCudaErrors(cudaMemcpy2D(
			d_temperature_out, d_pitch, d_temperature_in, d_pitch, meshSize * sizeof(float), meshSize, cudaMemcpyDeviceToDevice));

		for (int step = 0; step < steps; ++step) {
			meshUpdateKernel<<<gridSize, blockSize>>>(d_temperature_in, d_temperature_out, d_pitch, meshSize);
			checkCudaErrors(cudaGetLastError()); // Check for any errors launching the kernel
			checkCudaErrors(
				cudaDeviceSynchronize()); // cudaDeviceSynchronize waits for the kernel to finish, and returns any errors encountered during the launch.
			std::swap(d_temperature_in, d_temperature_out);
		}

		checkCudaErrors(
			cudaMemcpy2D(temperature, pitch, d_temperature_in, d_pitch, meshSize * sizeof(float), meshSize, cudaMemcpyDeviceToHost));
	}

	SimpleTimer t("Computation results processing");
	std::vector<std::vector<float>> result;
	for (int y = 1; y < meshSize - 1; ++y) {
		result.emplace_back();
		for (int x = 1; x < meshSize - 1; ++x) {
			result.back().emplace_back(*getElem(temperature, pitch, x, y));
		}
	}

	if (!validateResults(temperature, pitch)) { return {}; }

	//TODO: RAII guard to avoid resource leaks
	delete[] temperature;
	checkCudaErrors(cudaFree(d_temperature_in));
	checkCudaErrors(cudaFree(d_temperature_out));

	// cudaDeviceReset must be called before exiting in order for profiling and
	// tracing tools such as Nsight and Visual Profiler to show complete traces.
	checkCudaErrors(cudaDeviceReset());

	return result;
}