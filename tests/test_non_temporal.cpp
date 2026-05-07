#include <gtest/gtest.h>
#include <cstddef>
#include "aligned_alloc.hpp"
#include "benchmark.hpp"

TEST(NonTemporal, CopyFallbackFloat) {
    // When SIMD is not enabled, copy_kernel_nt falls back to copy_kernel
    // which should produce identical results.
    const std::size_t n = 128;
    const std::size_t total_bytes = n * sizeof(float);

    float* a = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    float* c = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));

    ASSERT_NE(a, nullptr);
    ASSERT_NE(c, nullptr);

    for (std::size_t i = 0; i < n; ++i) {
        a[i] = static_cast<float>(i * 1.5f);
    }

    mem_band::copy_kernel_nt<float>(a, c, n);

    float* expected = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    ASSERT_NE(expected, nullptr);
    mem_band::copy_kernel<float>(a, expected, n);

    for (std::size_t i = 0; i < n; ++i) {
        EXPECT_FLOAT_EQ(c[i], expected[i]);
    }

    mem_band::aligned_free(a);
    mem_band::aligned_free(c);
    mem_band::aligned_free(expected);
}

TEST(NonTemporal, CopyFallbackDouble) {
    const std::size_t n = 128;
    const std::size_t total_bytes = n * sizeof(double);

    double* a = static_cast<double*>(mem_band::aligned_alloc(total_bytes, 64));
    double* c = static_cast<double*>(mem_band::aligned_alloc(total_bytes, 64));

    ASSERT_NE(a, nullptr);
    ASSERT_NE(c, nullptr);

    for (std::size_t i = 0; i < n; ++i) {
        a[i] = static_cast<double>(i * 2.5);
    }

    mem_band::copy_kernel_nt<double>(a, c, n);

    double* expected = static_cast<double*>(mem_band::aligned_alloc(total_bytes, 64));
    ASSERT_NE(expected, nullptr);
    mem_band::copy_kernel<double>(a, expected, n);

    for (std::size_t i = 0; i < n; ++i) {
        EXPECT_DOUBLE_EQ(c[i], expected[i]);
    }

    mem_band::aligned_free(a);
    mem_band::aligned_free(c);
    mem_band::aligned_free(expected);
}

TEST(NonTemporal, TriadFallbackFloat) {
    const std::size_t n = 128;
    const std::size_t total_bytes = n * sizeof(float);
    const float scalar = 3.0f;

    float* a = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    float* b = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    float* c = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));

    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    ASSERT_NE(c, nullptr);

    for (std::size_t i = 0; i < n; ++i) {
        a[i] = static_cast<float>(i);
        b[i] = static_cast<float>(i * 2);
    }

    mem_band::triad_kernel_nt<float>(a, b, c, scalar, n);

    float* expected = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    ASSERT_NE(expected, nullptr);
    mem_band::triad_kernel<float>(a, b, expected, scalar, n);

    for (std::size_t i = 0; i < n; ++i) {
        float exp = a[i] + scalar * b[i];
        EXPECT_FLOAT_EQ(c[i], exp);
    }

    mem_band::aligned_free(a);
    mem_band::aligned_free(b);
    mem_band::aligned_free(c);
    mem_band::aligned_free(expected);
}

TEST(NonTemporal, TriadFallbackDouble) {
    const std::size_t n = 128;
    const std::size_t total_bytes = n * sizeof(double);
    const double scalar = 2.5;

    double* a = static_cast<double*>(mem_band::aligned_alloc(total_bytes, 64));
    double* b = static_cast<double*>(mem_band::aligned_alloc(total_bytes, 64));
    double* c = static_cast<double*>(mem_band::aligned_alloc(total_bytes, 64));

    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    ASSERT_NE(c, nullptr);

    for (std::size_t i = 0; i < n; ++i) {
        a[i] = static_cast<double>(i);
        b[i] = static_cast<double>(i + 1);
    }

    mem_band::triad_kernel_nt<double>(a, b, c, scalar, n);

    for (std::size_t i = 0; i < n; ++i) {
        double exp = a[i] + scalar * b[i];
        EXPECT_DOUBLE_EQ(c[i], exp);
    }

    mem_band::aligned_free(a);
    mem_band::aligned_free(b);
    mem_band::aligned_free(c);
}

TEST(NonTemporal, CopyNTSSE2Fallback) {
    // copy_kernel_nt_simd falls back to copy_kernel_nt (which falls back
    // to copy_kernel) when SIMD is not available, so results match.
    const std::size_t n = 128;
    const std::size_t total_bytes = n * sizeof(float);

    float* a = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    float* c_nt = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    float* c_ref = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));

    ASSERT_NE(a, nullptr);
    ASSERT_NE(c_nt, nullptr);
    ASSERT_NE(c_ref, nullptr);

    for (std::size_t i = 0; i < n; ++i) {
        a[i] = static_cast<float>(i);
    }

    mem_band::copy_kernel_nt_simd(a, c_nt, n);
    mem_band::copy_kernel<float>(a, c_ref, n);

    for (std::size_t i = 0; i < n; ++i) {
        EXPECT_FLOAT_EQ(c_nt[i], c_ref[i]);
    }

    mem_band::aligned_free(a);
    mem_band::aligned_free(c_nt);
    mem_band::aligned_free(c_ref);
}

TEST(NonTemporal, TriadNTSSE2Fallback) {
    const std::size_t n = 128;
    const std::size_t total_bytes = n * sizeof(float);
    const float scalar = 4.0f;

    float* a = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    float* b = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    float* c_nt = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    float* c_ref = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));

    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    ASSERT_NE(c_nt, nullptr);
    ASSERT_NE(c_ref, nullptr);

    for (std::size_t i = 0; i < n; ++i) {
        a[i] = static_cast<float>(i);
        b[i] = static_cast<float>((i + 1) * 2);
    }

    mem_band::triad_kernel_nt_simd(a, b, c_nt, scalar, n);
    mem_band::triad_kernel<float>(a, b, c_ref, scalar, n);

    for (std::size_t i = 0; i < n; ++i) {
        float exp = a[i] + scalar * b[i];
        EXPECT_FLOAT_EQ(c_nt[i], exp);
    }

    mem_band::aligned_free(a);
    mem_band::aligned_free(b);
    mem_band::aligned_free(c_nt);
    mem_band::aligned_free(c_ref);
}
