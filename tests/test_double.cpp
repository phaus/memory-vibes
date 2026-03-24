#include <cassert>
#include <cstddef>
#include "aligned_alloc.hpp"
#include "benchmark.hpp"

int main() {
    const std::size_t n = 8;
    const std::size_t total_bytes = n * sizeof(double);
    double* a = static_cast<double*>(mem_band::aligned_alloc(total_bytes, 64));
    double* b = static_cast<double*>(mem_band::aligned_alloc(total_bytes, 64));
    double* c = static_cast<double*>(mem_band::aligned_alloc(total_bytes, 64));
    assert(a && b && c);
    for (std::size_t i = 0; i < n; ++i) {
        a[i] = 1.0;
        b[i] = 2.0;
        c[i] = 0.0;
    }
    mem_band::copy_kernel<double>(a, c, n);
    for (std::size_t i = 0; i < n; ++i) {
        assert(c[i] == a[i]);
    }
    const double scalar = 3.0;
    mem_band::triad_kernel<double>(a, b, c, scalar, n);
    for (std::size_t i = 0; i < n; ++i) {
        assert(c[i] == a[i] + scalar * b[i]);
    }
    mem_band::aligned_free(a);
    mem_band::aligned_free(b);
    mem_band::aligned_free(c);
    return 0;
}
