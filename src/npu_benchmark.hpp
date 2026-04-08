// npu_benchmark.hpp
// Neural Processing Unit (NPU) benchmark framework.
// Provides mock NPU benchmarks for testing and benchmarking NPU accelerators.

#ifndef NPU_BENCHMARK_HPP
#define NPU_BENCHMARK_HPP

#include <string>
#include <cstdint>
#include <chrono>
#include <cmath>
#include <random>
#include <iostream>

namespace mem_band {

/**
 * @brief NPU supported data precision modes
 */
enum class NPUPrecision {
    FP32,  // IEEE 754 single precision
    FP16,  // IEEE 754 half precision
    FP8,   // 8-bit floating point
    INT8,  // 8-bit integer
    INT4   // 4-bit integer
};

/**
 * @brief Common NPU workload operation types
 */
enum class NPUOpType {
    MATMUL,  // Matrix multiplication
    CONV2D,  // 2D convolution
    RNN,     // Recurrent neural network
    ATTENTION  // Self-attention mechanism
};

/**
 * @brief NPU benchmark configuration
 */
struct NPUConfig {
    std::size_t workload_size = 1024;      // Size of input/output tensors
    std::size_t iterations = 10;           // Number of benchmark iterations
    NPUPrecision precision = NPUPrecision::FP32;
    NPUOpType op_type = NPUOpType::MATMUL;
    int device_id = 0;                     // Device identifier (for multi-device)
    
    // Convert enum to string
    static std::string precision_str(NPUPrecision p) {
        switch (p) {
            case NPUPrecision::FP32: return "FP32";
            case NPUPrecision::FP16: return "FP16";
            case NPUPrecision::FP8:  return "FP8";
            case NPUPrecision::INT8: return "INT8";
            case NPUPrecision::INT4: return "INT4";
            default: return "Unknown";
        }
    }
    
    static std::string op_type_str(NPUOpType op) {
        switch (op) {
            case NPUOpType::MATMUL: return "MatMul";
            case NPUOpType::CONV2D: return "Conv2D";
            case NPUOpType::RNN:    return "RNN";
            case NPUOpType::ATTENTION: return "Attention";
            default: return "Unknown";
        }
    }
};

/**
 * @brief NPU benchmark result
 */
struct NPUResult {
    double latency_ms;           // Average latency per operation
    double throughput_ops;       // Operations per second (OPS)
    double throughput_tflops;    // Tera operations per second
    double power_watts;          // Estimated power consumption
    int device_id;               // Device used for benchmark
};

/**
 * @brief Get the number of bytes per element for a given precision
 */
inline size_t bytes_per_element(NPUPrecision precision) {
    switch (precision) {
        case NPUPrecision::FP32: return 4;
        case NPUPrecision::FP16: return 2;
        case NPUPrecision::FP8:  return 1;
        case NPUPrecision::INT8: return 1;
        case NPUPrecision::INT4: return 0;
        default: return 4;
    }
}

/**
 * @brief Mock NPU benchmark implementation.
 * 
 * Since actual NPU access requires vendor-specific APIs (e.g., Apple Neural Engine,
 * Qualcomm Hexagon, Intel NPU), this provides a mock implementation that simulates
 * NPU behavior with configurable parameters.
 * 
 * @param config Benchmark configuration
 * @return NPUResult with benchmark metrics
 */
inline NPUResult mock_npu_benchmark(const NPUConfig& config) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> noise(0.95, 1.05);
    
    // Base performance characteristics for different precisions (relative to FP32)
    constexpr double perf_factors[] = {1.0, 4.0, 8.0, 16.0, 32.0};  // FP32, FP16, FP8, INT8, INT4
    constexpr double power_factors[] = {1.0, 0.5, 0.3, 0.2, 0.15};  // Relative power consumption
    
    // Base performance characteristics for different operations (relative to MatMul)
    constexpr double op_factors[] = {1.0, 0.8, 0.6, 0.4};  // MatMul, Conv2D, RNN, Attention
    
    size_t bytes = bytes_per_element(config.precision);
    double perf_factor = static_cast<double>(perf_factors[static_cast<int>(config.precision)]);
    double power_factor = static_cast<double>(power_factors[static_cast<int>(config.precision)]);
    double op_factor = static_cast<double>(op_factors[static_cast<int>(config.op_type)]);
    
    // Simulate computation time (mock: base latency + noise)
    double base_latency = 100.0;  // ms
    double total_time = 0.0;
    
    for (size_t i = 0; i < config.iterations; ++i) {
        double simulated_latency = base_latency * noise(gen) * 
                                   (1.0 / perf_factor) * 
                                   (1.0 / op_factor) *
                                   (1.0 + (config.workload_size / 10000.0));
        total_time += simulated_latency;
    }
    double avg_latency = total_time / config.iterations;
    
    // Calculate throughput: ops = workload_size * precision_elements
    // Simplified: assume 4 ops per element for matmul-like operations
    double ops_per_workload = config.workload_size * 4;
    double throughput_ops = ops_per_workload / (avg_latency / 1000.0);
    double throughput_tflops = throughput_ops / 1e12;
    
    // Estimate power consumption (mock: base power scaled by performance)
    double base_power = 5.0;  // Watts
    double power = base_power * power_factor;
    
    NPUResult result;
    result.latency_ms = avg_latency * noise(gen);
    result.throughput_ops = throughput_ops * noise(gen);
    result.throughput_tflops = throughput_tflops * noise(gen);
    result.power_watts = power * noise(gen);
    result.device_id = config.device_id;
    
    return result;
}

/**
 * @brief Run full NPU benchmark suite with various configurations
 * 
 * @param base_config Base configuration
 * @return Vector of results for all precision and operation combinations
 */
inline std::vector<NPUResult> run_npu_benchmark_suite(const NPUConfig& base_config) {
    std::vector<NPUResult> results;
    results.reserve(8);
    
    auto precisions = {NPUPrecision::FP32, NPUPrecision::FP16, NPUPrecision::INT8, NPUPrecision::INT4};
    auto op_types = {NPUOpType::MATMUL, NPUOpType::CONV2D};
    
    for (auto precision : precisions) {
        for (auto op_type : op_types) {
            NPUConfig config = base_config;
            config.precision = precision;
            config.op_type = op_type;
            results.push_back(mock_npu_benchmark(config));
        }
    }
    
    return results;
}

/**
 * @brief Print NPU benchmark result to stream
 * 
 * @param result Benchmark result
 * @param config Configuration used
 * @param os Output stream (default: stdout)
 */
inline void print_npu_result(const NPUResult& result, const NPUConfig& config, 
                             std::ostream& os = std::cout) {
    os << "# NPU Benchmark\n";
    os << "  Device: NPU " << result.device_id << "\n";
    os << "  Configuration: " << NPUConfig::op_type_str(config.op_type) 
       << " " << NPUConfig::precision_str(config.precision) 
       << " (size=" << config.workload_size << ")\n";
    os << "  Metrics:\n";
    os << "    Latency:      " << result.latency_ms << " ms\n";
    os << "    Throughput:   " << result.throughput_ops << " OPS\n";
    os << "                (" << result.throughput_tflops << " TFLOPS)\n";
    os << "    Power:        " << result.power_watts << " W\n";
}

/**
 * @brief Print NPU benchmark suite results to stream
 * 
 * @param results Benchmark results
 * @param os Output stream (default: stdout)
 */
inline void print_npu_suite_results(const std::vector<NPUResult>& results,
                                    std::ostream& os = std::cout) {
    os << "# NPU Benchmark Suite Results\n";
    os << "Precision | OpType      | Latency(ms) | OPS          | TFLOPS   | Power(W)\n";
    os << "----------|-------------|-------------|--------------|----------|---------\n";
    
    for (const auto& result : results) {
        os << "";
    }
    os << std::endl;
}

} // namespace mem_band

#endif // NPU_BENCHMARK_HPP
