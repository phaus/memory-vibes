#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "aligned_alloc.hpp"
#include "benchmark.hpp"

TEST(MultiThread, CopyKernel) {
    const std::size_t n = 1000000;
    const std::size_t total_bytes = n * sizeof(float);
    
    float* a = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    float* c = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    
    ASSERT_NE(a, nullptr);
    ASSERT_NE(c, nullptr);
    
    for (std::size_t i = 0; i < n; ++i) {
        a[i] = static_cast<float>(i % 100);
    }
    
    mem_band::copy_kernel(a, c, n);
    
    for (std::size_t i = 0; i < n; ++i) {
        EXPECT_EQ(c[i], a[i]);
    }
    
    mem_band::aligned_free(a);
    mem_band::aligned_free(c);
}

TEST(MultiThread, TriadKernel) {
    const std::size_t n = 1000000;
    const std::size_t total_bytes = n * sizeof(float);
    
    float* a = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    float* b = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    float* c = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    ASSERT_NE(c, nullptr);
    
    const float scalar = 2.5f;
    
    for (std::size_t i = 0; i < n; ++i) {
        a[i] = static_cast<float>(i % 100);
        b[i] = static_cast<float>((i + 1) % 50);
    }
    
    mem_band::triad_kernel(a, b, c, scalar, n);
    
    for (std::size_t i = 0; i < n; ++i) {
        float expected = a[i] + scalar * b[i];
        EXPECT_FLOAT_EQ(c[i], expected);
    }
    
    mem_band::aligned_free(a);
    mem_band::aligned_free(b);
    mem_band::aligned_free(c);
}

TEST(MultiThread, ScaleKernel) {
    const std::size_t n = 1000000;
    const std::size_t total_bytes = n * sizeof(float);
    
    float* a = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    float* c = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    
    ASSERT_NE(a, nullptr);
    ASSERT_NE(c, nullptr);
    
    const float scalar = 3.14159f;
    
    for (std::size_t i = 0; i < n; ++i) {
        a[i] = static_cast<float>(i % 100);
    }
    
    mem_band::scale_kernel(a, c, scalar, n);
    
    for (std::size_t i = 0; i < n; ++i) {
        float expected = scalar * a[i];
        EXPECT_FLOAT_EQ(c[i], expected);
    }
    
    mem_band::aligned_free(a);
    mem_band::aligned_free(c);
}

TEST(MultiThread, AddKernel) {
    const std::size_t n = 1000000;
    const std::size_t total_bytes = n * sizeof(float);
    
    float* a = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    float* b = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    float* c = static_cast<float*>(mem_band::aligned_alloc(total_bytes, 64));
    
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    ASSERT_NE(c, nullptr);
    
    for (std::size_t i = 0; i < n; ++i) {
        a[i] = static_cast<float>(i % 100);
        b[i] = static_cast<float>((i + 1) % 50);
    }
    
    mem_band::add_kernel(a, b, c, n);
    
    for (std::size_t i = 0; i < n; ++i) {
        float expected = a[i] + b[i];
        EXPECT_FLOAT_EQ(c[i], expected);
    }
    
    mem_band::aligned_free(a);
    mem_band::aligned_free(b);
    mem_band::aligned_free(c);
}

TEST(MultiThread, DoublePrecision) {
    const std::size_t n = 1000000;
    const std::size_t total_bytes = n * sizeof(double);
    
    double* a = static_cast<double*>(mem_band::aligned_alloc(total_bytes, 64));
    double* b = static_cast<double*>(mem_band::aligned_alloc(total_bytes, 64));
    double* c = static_cast<double*>(mem_band::aligned_alloc(total_bytes, 64));
    
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    ASSERT_NE(c, nullptr);
    
    const double scalar = 1.5;
    
    for (std::size_t i = 0; i < n; ++i) {
        a[i] = static_cast<double>(i % 100);
        b[i] = static_cast<double>((i + 1) % 50);
    }
    
    mem_band::triad_kernel(a, b, c, scalar, n);
    
    for (std::size_t i = 0; i < n; ++i) {
        double expected = a[i] + scalar * b[i];
        EXPECT_DOUBLE_EQ(c[i], expected);
    }
    
    mem_band::aligned_free(a);
    mem_band::aligned_free(b);
    mem_band::aligned_free(c);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
