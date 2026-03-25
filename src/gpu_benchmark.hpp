// gpu_benchmark.hpp
// Simple GPU copy kernel for float (CUDA)

#ifndef GPU_BENCHMARK_HPP
#define GPU_BENCHMARK_HPP

#include <iostream>

#ifdef __CUDACC__
#include <cuda_runtime.h>
// Copy kernel: dst[i] = src[i]
__global__ void copy_kernel(const float* src, float* dst, size_t n) {
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        dst[idx] = src[idx];
    }
}
#endif

// Host function to launch the copy kernel and measure time
inline void copy_kernel_benchmark(const float* h_src, float* h_dst, size_t n, float& elapsed_time_ms) {
#ifdef __CUDACC__
    // Allocate device memory
    float *d_src = nullptr, *d_dst = nullptr;
    size_t bytes = n * sizeof(float);
    cudaError_t err = cudaMalloc(&d_src, bytes);
    if (err != cudaSuccess) {
        std::cerr << "cudaMalloc for d_src failed: " << cudaGetErrorString(err) << std::endl;
        return;
    }
    err = cudaMalloc(&d_dst, bytes);
    if (err != cudaSuccess) {
        std::cerr << "cudaMalloc for d_dst failed: " << cudaGetErrorString(err) << std::endl;
        cudaFree(d_src);
        return;
    }

    // Copy host source to device source
    err = cudaMemcpy(d_src, h_src, bytes, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        std::cerr << "cudaMemcpy H2D for d_src failed: " << cudaGetErrorString(err) << std::endl;
        cudaFree(d_src);
        cudaFree(d_dst);
        return;
    }

    // Launch kernel
    const int blockSize = 256;
    const int gridSize = (n + blockSize - 1) / blockSize;
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start, 0);

    copy_kernel<<<gridSize, blockSize>>>(d_src, d_dst, n);
    err = cudaGetLastError();
    if (err != cudaSuccess) {
        std::cerr << "Kernel launch failed: " << cudaGetErrorString(err) << std::endl;
        cudaEventDestroy(start);
        cudaEventDestroy(stop);
        cudaFree(d_src);
        cudaFree(d_dst);
        return;
    }

    cudaEventRecord(stop, 0);
    cudaEventSynchronize(stop);
    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);
    elapsed_time_ms = milliseconds;

    // Copy result back to host
    err = cudaMemcpy(h_dst, d_dst, bytes, cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) {
        std::cerr << "cudaMemcpy D2H failed: " << cudaGetErrorString(err) << std::endl;
    }

    // Cleanup
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    cudaFree(d_src);
    cudaFree(d_dst);
#else
    // Fallback for non-CUDA compilers: do nothing and set time to zero
    elapsed_time_ms = 0.0f;
    std::cerr << "Warning: GPU benchmark requested but CUDA not available. Returning zero time." << std::endl;
#endif // __CUDACC__
}
#endif // GPU_BENCHMARK_HPP