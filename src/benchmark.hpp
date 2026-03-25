// benchmark.hpp
// SIMD optional kernels (enabled with -S/--simd when compiled with AVX2)
// Simple vectorized implementations for float and double.
// Note: These are basic examples and may not be optimal for all CPUs.
// Templated STREAM benchmark kernels (Copy and Triad).
// Implemented as simple loops operating on raw pointers.

#ifndef BENCHMARK_HPP

#if defined(SIMD_ENABLED) && defined(__AVX2__)
#include <immintrin.h> // SIMD intrinsics (AVX2)
#endif
#define BENCHMARK_HPP

#include <type_traits>
#include <algorithm>
#include <cmath>
#include <vector>
#include <random>

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

// SIMD kernels (AVX2). Enabled when compiled with -mavx2 and -DSIMD_ENABLED.
#if defined(SIMD_ENABLED) && defined(__AVX2__)

inline void copy_kernel_simd(const float* a, float* c, std::size_t n) {
    std::size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256 v = _mm256_load_ps(a + i);
        _mm256_store_ps(c + i, v);
    }
    for (; i < n; ++i) {
        c[i] = a[i];
    }
}

inline void copy_kernel_simd(const double* a, double* c, std::size_t n) {
    std::size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        __m256d v = _mm256_load_pd(a + i);
        _mm256_store_pd(c + i, v);
    }
    for (; i < n; ++i) {
        c[i] = a[i];
    }
}

inline void triad_kernel_simd(const float* a, const float* b, float* c, float scalar, std::size_t n) {
    __m256 s = _mm256_set1_ps(scalar);
    std::size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256 av = _mm256_load_ps(a + i);
        __m256 bv = _mm256_load_ps(b + i);
        __m256 mul = _mm256_mul_ps(s, bv);
    __m256 res = _mm256_add_ps(av, mul); // av + scalar * bv
        _mm256_store_ps(c + i, res);
    }
    for (; i < n; ++i) {
        c[i] = a[i] + scalar * b[i];
    }
}

inline void triad_kernel_simd(const double* a, const double* b, double* c, double scalar, std::size_t n) {
    __m256d s = _mm256_set1_pd(scalar);
    std::size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        __m256d av = _mm256_load_pd(a + i);
        __m256d bv = _mm256_load_pd(b + i);
        __m256d mul = _mm256_mul_pd(s, bv);
        __m256d res = _mm256_add_pd(av, mul);
        _mm256_store_pd(c + i, res);
    }
    for (; i < n; ++i) {
        c[i] = a[i] + scalar * b[i];
    }
}
#else
// Fallback definitions when SIMD is not enabled – map to scalar kernels
inline void copy_kernel_simd(const float* a, float* c, std::size_t n) { copy_kernel<float>(a, c, n); }
inline void copy_kernel_simd(const double* a, double* c, std::size_t n) { copy_kernel<double>(a, c, n); }

inline void triad_kernel_simd(const float* a, const float* b, float* c, float scalar, std::size_t n) { triad_kernel<float>(a, b, c, scalar, n); }
inline void triad_kernel_simd(const double* a, const double* b, double* c, double scalar, std::size_t n) { triad_kernel<double>(a, b, c, scalar, n); }
#endif // SIMD_ENABLED && __AVX2__

// Scale kernel: c[i] = scalar * a[i]
template <typename T>
void scale_kernel(const T* a, T* c, T scalar, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        c[i] = scalar * a[i];
    }
}

// Add kernel: c[i] = a[i] + b[i]
template <typename T>
void add_kernel(const T* a, const T* b, T* c, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}


template <typename T>
void random_rw_kernel(const T* a, T* c, std::size_t n) {
    // Create a vector of indices [0, n)
    std::vector<std::size_t> idx(n);
    for (std::size_t i = 0; i < n; ++i) idx[i] = i;
    // Shuffle indices
    std::mt19937 rng(0xDEADBEEF); // deterministic seed
    std::shuffle(idx.begin(), idx.end(), rng);
    // Perform random accesses
    for (std::size_t i = 0; i < n; ++i) {
        c[idx[i]] = a[idx[i]];
    }
}

} // namespace mem_band

#endif // BENCHMARK_HPP