#include <cassert>
#include <cstddef>
#include "aligned_alloc.hpp"
#include "benchmark.hpp"

int main() {
    // Allocate small arrays
    const std::size_t n = 1024;
    float* a = static_cast<float*>(mem_band::aligned_alloc(n * sizeof(float), 64));
    float* b = static_cast<float*>(mem_band::aligned_alloc(n * sizeof(float), 64));
    float* c = static_cast<float*>(mem_band::aligned_alloc(n * sizeof(float), 64));
    assert(a != nullptr && b != nullptr && c != nullptr);
    // Initialize
    for (std::size_t i = 0; i < n; ++i) {
        a[i] = 1.0f;
        b[i] = 2.0f;
        c[i] = 3.0f;
    }
    // Run the ALU kernel
    mem_band::alu_kernel(a, b, c, n);
    // Check that the arrays are modified (we expect a[i] to be changed)
    bool changed = false;
    for (std::size_t i = 0; i < n; ++i) {
        // We don't know the exact value, but we know it's not the initial value?
        // Actually, we do: 
        //   temp = a[i] * b[i] + c[i]
        //   a[i] = temp * c[i] + temp
        //   = temp * (c[i] + 1)
        //   = (a[i] * b[i] + c[i]) * (c[i] + 1)
        // With initial values: a[i]=1, b[i]=2, c[i]=3
        // temp = 1*2 + 3 = 5
        // a[i] = 5 * (3 + 1) = 5 * 4 = 20
        if (a[i] != 20.0f) {
            changed = true;
            break;
        }
    }
    assert(changed);
    mem_band::aligned_free(a);
    mem_band::aligned_free(b);
    mem_band::aligned_free(c);
    return 0;
}