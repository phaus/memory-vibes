// ssd_benchmark.hpp
// SSD I/O benchmark kernels (Sequential and Random read/write tests)

#ifndef SSD_BENCHMARK_HPP
#define SSD_BENCHMARK_HPP

#include <fstream>
#include <random>
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>
#include <cstring>
#include <memory>
#include <algorithm>

namespace mem_band {

// SSD I/O benchmark configuration
struct SSDConfig {
    std::string path;
    std::size_t block_size;      // Block size in bytes (1kB-4kB typical)
    std::size_t num_blocks;      // Number of blocks
    bool sequential;             // Sequential or random I/O
    bool read_only;              // Read-only or read-write benchmark
};

// SSD benchmark result
struct SSDResult {
    double bandwidth_mbps;
    double iops;
    double latency_us;
    std::size_t bytes_transferred;
};

// Generate random file path for benchmark
inline std::string generate_ssd_file_path(const std::string& base_path, const std::string& suffix) {
    return base_path + "/mem_band_ssd_" + suffix;
}

// Sequential write benchmark
// Measures sequential write throughput
inline SSDResult sequential_write_benchmark(const std::string& file_path, std::size_t block_size, std::size_t num_blocks) {
    SSDResult result{};
    const std::uint64_t total_bytes = block_size * num_blocks;
    result.bytes_transferred = total_bytes;
    
    std::vector<char> buffer(block_size, 0xAA);
    
    std::ofstream outfile(file_path, std::ios::binary | std::ios::trunc);
    if (!outfile) {
        result.bandwidth_mbps = 0.0;
        result.iops = 0.0;
        result.latency_us = 0.0;
        return result;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (std::size_t i = 0; i < num_blocks; ++i) {
        outfile.write(buffer.data(), block_size);
        if (!outfile) {
            break;
        }
    }
    
    outfile.flush();
    outfile.close();
    
    auto end = std::chrono::high_resolution_clock::now();
    double duration_s = std::chrono::duration<double>(end - start).count();
    
    if (duration_s > 0) {
        result.bandwidth_mbps = (total_bytes / (1024.0 * 1024.0)) / duration_s * 1000; // MB/s -> *1000 for Mbps equivalent
        result.iops = num_blocks / duration_s;
        result.latency_us = (duration_s / num_blocks) * 1000000; // seconds to microseconds
    }
    
    return result;
}

// Sequential read benchmark
// Measures sequential read throughput
inline SSDResult sequential_read_benchmark(const std::string& file_path, std::size_t block_size, std::size_t num_blocks) {
    SSDResult result{};
    const std::uint64_t total_bytes = block_size * num_blocks;
    result.bytes_transferred = total_bytes;
    
    std::vector<char> buffer(block_size);
    
    std::ifstream infile(file_path, std::ios::binary);
    if (!infile) {
        result.bandwidth_mbps = 0.0;
        result.iops = 0.0;
        result.latency_us = 0.0;
        return result;
    }
    
    // Prefetch first block
    infile.read(buffer.data(), block_size);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (std::size_t i = 1; i < num_blocks; ++i) {
        infile.read(buffer.data(), block_size);
        if (!infile) {
            break;
        }
        // Prevent optimization
        volatile char dummy = buffer[0];
        (void)dummy;
    }
    
    infile.close();
    
    auto end = std::chrono::high_resolution_clock::now();
    double duration_s = std::chrono::duration<double>(end - start).count();
    
    if (duration_s > 0) {
        result.bandwidth_mbps = (total_bytes / (1024.0 * 1024.0)) / duration_s * 1000;
        result.iops = num_blocks / duration_s;
        result.latency_us = (duration_s / num_blocks) * 1000000;
    }
    
    return result;
}

// Random write benchmark (less common, but useful for certain workloads)
// Note: This can be I/O heavy and slow
inline SSDResult random_write_benchmark(const std::string& file_path, std::size_t block_size, std::size_t num_blocks) {
    SSDResult result{};
    const std::uint64_t total_bytes = block_size * num_blocks;
    result.bytes_transferred = total_bytes;
    
    std::vector<char> buffer(block_size, 0xBB);
    
    // Pre-create file with sequential write first
    std::ofstream create_file(file_path, std::ios::binary | std::ios::trunc);
    if (!create_file) {
        result.bandwidth_mbps = 0.0;
        result.iops = 0.0;
        result.latency_us = 0.0;
        return result;
    }
    create_file.seekp((num_blocks - 1) * block_size);
    create_file.put('x');
    create_file.close();
    
    std::ofstream outfile(file_path, std::ios::binary);
    if (!outfile) {
        result.bandwidth_mbps = 0.0;
        result.iops = 0.0;
        result.latency_us = 0.0;
        return result;
    }
    
    // Generate random positions
    std::vector<std::size_t> positions(num_blocks);
    for (std::size_t i = 0; i < num_blocks; ++i) {
        positions[i] = i % num_blocks;
    }
    
    std::mt19937 rng(42);
    std::shuffle(positions.begin(), positions.end(), rng);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (std::size_t i = 0; i < num_blocks; ++i) {
        outfile.seekp(positions[i] * block_size);
        outfile.write(buffer.data(), block_size);
        if (!outfile) {
            break;
        }
    }
    
    outfile.flush();
    outfile.close();
    
    auto end = std::chrono::high_resolution_clock::now();
    double duration_s = std::chrono::duration<double>(end - start).count();
    
    if (duration_s > 0) {
        result.bandwidth_mbps = (total_bytes / (1024.0 * 1024.0)) / duration_s * 1000;
        result.iops = num_blocks / duration_s;
        result.latency_us = (duration_s / num_blocks) * 1000000;
    }
    
    return result;
}

// Random read benchmark
// Measures random read IOPS and throughput
inline SSDResult random_read_benchmark(const std::string& file_path, std::size_t block_size, std::size_t num_blocks) {
    SSDResult result{};
    const std::uint64_t total_bytes = block_size * num_blocks;
    result.bytes_transferred = total_bytes;
    
    std::vector<char> buffer(block_size);
    
    std::ifstream infile(file_path, std::ios::binary);
    if (!infile) {
        result.bandwidth_mbps = 0.0;
        result.iops = 0.0;
        result.latency_us = 0.0;
        return result;
    }
    
    // Generate random positions
    std::vector<std::size_t> positions(num_blocks);
    for (std::size_t i = 0; i < num_blocks; ++i) {
        positions[i] = i;
    }
    
    std::mt19937 rng(42);
    std::shuffle(positions.begin(), positions.end(), rng);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (std::size_t i = 0; i < num_blocks; ++i) {
        infile.seekg(positions[i] * block_size);
        infile.read(buffer.data(), block_size);
        if (!infile) {
            break;
        }
        // Prevent optimization
        volatile char dummy = buffer[0];
        (void)dummy;
    }
    
    infile.close();
    
    auto end = std::chrono::high_resolution_clock::now();
    double duration_s = std::chrono::duration<double>(end - start).count();
    
    if (duration_s > 0) {
        result.bandwidth_mbps = (total_bytes / (1024.0 * 1024.0)) / duration_s * 1000;
        result.iops = num_blocks / duration_s;
        result.latency_us = (duration_s / num_blocks) * 1000000;
    }
    
    return result;
}

// Test file cleanup
inline void cleanup_ssd_test_file(const std::string& file_path) {
    std::remove(file_path.c_str());
}

// Run SSD benchmark
// Returns empty result on error
inline SSDResult run_ssd_benchmark(const SSDConfig& config) {
    SSDResult result{};
    
    // Generate test file path
    std::string file_path = generate_ssd_file_path(config.path, "test");
    
    // Pre-create file if it doesn't exist
    if (config.read_only) {
        // File should already exist for read-only tests
        // Run benchmarks
        if (config.sequential) {
            result = sequential_read_benchmark(file_path, config.block_size, config.num_blocks);
        } else {
            result = random_read_benchmark(file_path, config.block_size, config.num_blocks);
        }
    } else {
        // Write-first approach
        if (config.sequential) {
            sequential_write_benchmark(file_path, config.block_size, config.num_blocks);
            result = sequential_read_benchmark(file_path, config.block_size, config.num_blocks);
        } else {
            random_write_benchmark(file_path, config.block_size, config.num_blocks);
            result = random_read_benchmark(file_path, config.block_size, config.num_blocks);
        }
    }
    
    // Cleanup
    cleanup_ssd_test_file(file_path);
    
    return result;
}

} // namespace mem_band

#endif // SSD_BENCHMARK_HPP
