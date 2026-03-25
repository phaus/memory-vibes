#include <iostream>
#include <cstdlib>
#include <cuda_runtime.h>
#include "../src/gpu_benchmark.hpp"

int main() {
    const size_t n = 1024;
    const size_t bytes = n * sizeof(float);

    // Allocate host arrays
    float* h_src = new float[n];
    float* h_dst = new float[n];

    // Initialize host source array
    for (size_t i = 0; i < n; ++i) {
        h_src[i] = static_cast<float>(i);
        h_dst[i] = 0.0f;
    }

    // Measure time for GPU copy
    float elapsed_time_ms = 0.0f;
    copy_kernel_benchmark(h_src, h_dst, n, elapsed_time_ms);

    // Check result
    bool passed = true;
    for (size_t i = 0; i < n; ++i) {
        if (h_dst[i] != h_src[i]) {
            passed = false;
            break;
        }
    }

    // Cleanup
    delete[] h_src;
    delete[] h_dst;

    if (passed) {
        std::cout << "GPU copy test passed. Time: " << elapsed_time_ms << " ms" << std::endl;
        return 0;
    } else {
        std::cerr << "GPU copy test failed: data mismatch!" << std::endl;
        return 1;
    }
}