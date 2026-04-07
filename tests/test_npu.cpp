#include <cassert>
#include <iostream>
#include "npu_benchmark.hpp"

int main() {
    // Test default configuration
    mem_band::NPUConfig config;
    assert(config.workload_size == 1024);
    assert(config.iterations == 10);
    assert(config.precision == mem_band::NPUPrecision::FP32);
    assert(config.op_type == mem_band::NPUOpType::MATMUL);
    assert(config.device_id == 0);
    std::cout << "Default config test passed\n";

    // Test custom configuration
    mem_band::NPUConfig custom_config;
    custom_config.workload_size = 2048;
    custom_config.iterations = 20;
    custom_config.precision = mem_band::NPUPrecision::FP16;
    custom_config.op_type = mem_band::NPUOpType::CONV2D;
    custom_config.device_id = 1;
    assert(custom_config.workload_size == 2048);
    assert(custom_config.iterations == 20);
    assert(custom_config.precision == mem_band::NPUPrecision::FP16);
    assert(custom_config.op_type == mem_band::NPUOpType::CONV2D);
    assert(custom_config.device_id == 1);
    std::cout << "Custom config test passed\n";

    // Test mock NPU benchmark
    mem_band::NPUResult result = mem_band::mock_npu_benchmark(custom_config);
    assert(result.latency_ms > 0);
    assert(result.throughput_ops > 0);
    assert(result.power_watts >= 0);
    std::cout << "Mock benchmark test passed\n";
    std::cout << "  Latency: " << result.latency_ms << " ms\n";
    std::cout << "  Throughput: " << result.throughput_ops << " ops/s\n";
    std::cout << "  Power: " << result.power_watts << " W\n";

    // Test different precision modes
    for (auto precision : {mem_band::NPUPrecision::FP32, mem_band::NPUPrecision::FP16, 
                           mem_band::NPUPrecision::INT8, mem_band::NPUPrecision::INT4}) {
        custom_config.precision = precision;
        result = mem_band::mock_npu_benchmark(custom_config);
        assert(result.latency_ms > 0);
    }
    std::cout << "Precision variations test passed\n";

    // Test different operation types
    for (auto op_type : {mem_band::NPUOpType::MATMUL, mem_band::NPUOpType::CONV2D,
                         mem_band::NPUOpType::RNN, mem_band::NPUOpType::ATTENTION}) {
        custom_config.op_type = op_type;
        result = mem_band::mock_npu_benchmark(custom_config);
        assert(result.latency_ms > 0);
    }
    std::cout << "Operation type variations test passed\n";

    // Print summary
    std::cout << "\nAll NPU benchmark tests passed!\n";
    return 0;
}
