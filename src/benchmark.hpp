// benchmark.hpp
// Templated STREAM benchmark kernels (Copy and Triad).
// Implemented as simple loops operating on raw pointers.

#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

#include <cstddef>

namespace mem_band {

// Copy kernel: c[i] = a[i]
template <typename T>
void copy_kernel(const T* a, T* c, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        c[i] = a[i];
    }
}

// Triad kernel: c[i] = a[i] + scalar * b[i]
template <typename T>
void triad_kernel(const T* a, const T* b, T* c, T scalar, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        c[i] = a[i] + scalar * b[i];
    }
}

} // namespace mem_band

#endif // BENCHMARK_HPP