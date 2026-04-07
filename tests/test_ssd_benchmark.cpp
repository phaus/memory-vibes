// test_ssd_benchmark.cpp
// Unit tests for SSD I/O benchmark functionality

#include <iostream>
#include <cassert>
#include <cmath>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include "../src/ssd_benchmark.hpp"

namespace fs = std::filesystem;

// Helper macro for assertions
#define ASSERT_TRUE(cond) assert(cond)
#define ASSERT_FALSE(cond) assert(!(cond))
#define EXPECT_GT(val, min) do { if (!(val > min)) { std::cerr << "FAILED: " << #val  << " (" <<  (val) << ") > " << #min << " (" << (min) << ") failed\n"; exit(1); } } while(0)
#define EXPECT_LT(val, max) do { if (!(val < max)) { std::cerr << "FAILED: " << #val  << " (" <<  (val) << ") < " << #max << " (" << (max) << ") failed\n"; exit(1); } } while(0)

// Test sequential write benchmark
void TestSequentialWrite() {
    fs::path test_dir_path = fs::temp_directory_path() / "mem_band_ssd_test";
    fs::create_directories(test_dir_path);
    
    mem_band::SSDConfig config{};
    config.path = test_dir_path.string();
    config.block_size = 4096;  // 4KB block
    config.num_blocks = 100;
    config.sequential = true;
    config.read_only = false;
    
    auto result = mem_band::run_ssd_benchmark(config);
    
    // Verify result structure is valid
    EXPECT_GT(result.bytes_transferred, 0);
    
    // Cleanup done by function
}

// Test sequential read benchmark (with pre-existing data)
void TestSequentialRead() {
    std::string test_dir = fs::temp_directory_path() / "mem_band_ssd_test";
    fs::create_directories(test_dir);
    
    // First create data
    mem_band::SSDConfig create_config{};
    create_config.path = test_dir;
    create_config.block_size = 4096;
    create_config.num_blocks = 100;
    create_config.sequential = true;
    create_config.read_only = false;
    
    mem_band::run_ssd_benchmark(create_config);
    
    // Now read it back
    mem_band::SSDConfig read_config{};
    read_config.path = test_dir;
    read_config.block_size = 4096;
    read_config.num_blocks = 100;
    read_config.sequential = true;
    read_config.read_only = true;
    
    auto result = mem_band::run_ssd_benchmark(read_config);
    
    EXPECT_GT(result.bytes_transferred, 0);
}

// Test random read benchmark
void TestRandomRead() {
    std::string test_dir = fs::temp_directory_path() / "mem_band_ssd_test";
    fs::create_directories(test_dir);
    
    // First create data
    mem_band::SSDConfig create_config{};
    create_config.path = test_dir;
    create_config.block_size = 4096;
    create_config.num_blocks = 100;
    create_config.sequential = true;
    create_config.read_only = false;
    
    mem_band::run_ssd_benchmark(create_config);
    
    // Now read randomly
    mem_band::SSDConfig read_config{};
    read_config.path = test_dir;
    read_config.block_size = 4096;
    read_config.num_blocks = 100;
    read_config.sequential = false;
    read_config.read_only = true;
    
    auto result = mem_band::run_ssd_benchmark(read_config);
    
    EXPECT_GT(result.bytes_transferred, 0);
}

// Test with small blocks (1KB)
void TestSmallBlocks() {
    fs::path test_dir_path = fs::temp_directory_path() / "mem_band_ssd_test";
    fs::create_directories(test_dir_path);
    
    mem_band::SSDConfig config{};
    config.path = test_dir_path.string();
    config.block_size = 1024;   // 1KB block
    config.num_blocks = 200;
    config.sequential = true;
    config.read_only = false;
    
    auto result = mem_band::run_ssd_benchmark(config);
    
    EXPECT_GT(result.bytes_transferred, 0);
    EXPECT_GT(result.iops, 0);
}

// Test with large blocks (4KB)
void TestLargeBlocks() {
    fs::path test_dir_path = fs::temp_directory_path() / "mem_band_ssd_test";
    fs::create_directories(test_dir_path);
    
    mem_band::SSDConfig config{};
    config.path = test_dir_path.string();
    config.block_size = 4096;   // 4KB block
    config.num_blocks = 50;
    config.sequential = true;
    config.read_only = false;
    
    auto result = mem_band::run_ssd_benchmark(config);
    
    EXPECT_GT(result.bytes_transferred, 0);
    EXPECT_GT(result.bandwidth_mbps, 0);
}

// Test file cleanup
void TestCleanup() {
    fs::path test_dir_path = fs::temp_directory_path() / "mem_band_ssd_test";
    fs::create_directories(test_dir_path);
    
    mem_band::SSDConfig config{};
    config.path = test_dir_path.string();
    config.block_size = 4096;
    config.num_blocks = 10;
    config.sequential = true;
    config.read_only = false;
    
    auto result = mem_band::run_ssd_benchmark(config);
    
    // File should be cleaned up
    std::string file_path = config.path + "/mem_band_ssd_test";
    ASSERT_FALSE(fs::exists(file_path.c_str()));
}

// Test with different block sizes
void TestVaryingBlockSizes() {
    fs::path test_dir_path = fs::temp_directory_path() / "mem_band_ssd_test";
    fs::create_directories(test_dir_path);
    
    std::vector<std::size_t> block_sizes;
    block_sizes.push_back(1024);
    block_sizes.push_back(2048);
    block_sizes.push_back(4096);
    
    for (const auto& block_size : block_sizes) {
        mem_band::SSDConfig config{};
        config.path = test_dir_path.string();
        config.block_size = block_size;
        config.num_blocks = 50;
        config.sequential = true;
        config.read_only = false;
        
        auto result = mem_band::run_ssd_benchmark(config);
        
        // Verify result structure is valid
        EXPECT_GT(result.bytes_transferred, 0);
        EXPECT_GT(result.iops, 0);
    }
}

// Test IOPS calculation
void TestIOPS() {
    fs::path test_dir_path = fs::temp_directory_path() / "mem_band_ssd_test";
    fs::create_directories(test_dir_path);
    
    mem_band::SSDConfig config{};
    config.path = test_dir_path.string();
    config.block_size = 4096;
    config.num_blocks = 100;
    config.sequential = true;
    config.read_only = false;
    
    auto result = mem_band::run_ssd_benchmark(config);
    
    // IOPS should be positive
    EXPECT_GT(result.iops, 0);
}

// Test latency calculation
void TestLatency() {
    fs::path test_dir_path = fs::temp_directory_path() / "mem_band_ssd_test";
    fs::create_directories(test_dir_path);
    
    mem_band::SSDConfig config{};
    config.path = test_dir_path.string();
    config.block_size = 4096;
    config.num_blocks = 50;
    config.sequential = true;
    config.read_only = false;
    
    auto result = mem_band::run_ssd_benchmark(config);
    
    // Latency should be positive (can be < 1us for fast SSDs)
    EXPECT_GT(result.latency_us, 0);
    EXPECT_GT(result.latency_us, 0.001); // At least 1 nanosecond
}

int main() {
    std::cout << "Running SSD I/O Benchmark Tests..." << std::endl << std::endl;
    
    std::cout << "Test 1: Sequential Write... ";
    TestSequentialWrite();
    
    std::cout << "Test 2: Sequential Read... ";
    TestSequentialRead();
    
    std::cout << "Test 3: Random Read... ";
    TestRandomRead();
    
    std::cout << "Test 4: Small Blocks (1KB)... ";
    TestSmallBlocks();
    
    std::cout << "Test 5: Large Blocks (4KB)... ";
    TestLargeBlocks();
    
    std::cout << "Test 6: File Cleanup... ";
    TestCleanup();
    
    std::cout << "Test 7: Varying Block Sizes... ";
    TestVaryingBlockSizes();
    
    std::cout << "Test 8: IOPS Calculation... ";
    TestIOPS();
    
    std::cout << "Test 9: Latency Calculation... ";
    TestLatency();
    
    std::cout << std::endl << "All SSD benchmark tests PASSED!" << std::endl;
    return 0;
}
