#include <cassert>
#include <cstddef>
#include "aligned_alloc.hpp"
#include "benchmark.hpp"

int main() {
    const std::size_t n = 8;
    const std::size_t total_bytes = n * sizeof(float);
    float* a = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    float* b = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    float* c = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    assert(a && b && c);
    for (std::size_t i = 0; i < n; ++i) {
        a[i] = 1.0f;
        b[i] = 2.0f;
        c[i] = 0.0f;
    }
    mem_band::copy_kernel<float>(a, c, n);
    for (std::size_t i = 0; i < n; ++i) {
        assert(c[i] == a[i]);
    }
    const float scalar = 3.0f;
    mem_band::triad_kernel<float>(a, b, c, scalar, n);
    for (std::size_t i = 0; i < n; ++i) {
        assert(c[i] == a[i] + scalar * b[i]);
    }
    mem_band::aligned_free(a);
    mem_band::aligned_free(b);
    mem_band::aligned_free(c);
    return 0;
}
