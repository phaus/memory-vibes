// main.cpp
// Command‑line interface for the memory bandwidth benchmark.
// Parses options, allocates aligned arrays, runs copy and triad kernels,
// measures execution time, and prints CSV‑friendly results.

#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <chrono>

#include "aligned_alloc.hpp"
#include "benchmark.hpp"
#include "ssd_benchmark.hpp"
#include "apu_identifier.hpp"
#include "npu_benchmark.hpp"
#include "platform_detection.hpp"
#include "system_info.hpp"
#include "runtime_detection.hpp"
#include "json_output.hpp"
#include "layout_builder.hpp"

using namespace mem_band;

struct Options {
    std::size_t sizeMiB = 256;      // array size in MiB
    std::size_t iterations = 20;    // timed iterations per kernel
    std::string type = "float";     // data type: float or double
    bool simd = false;              // placeholder – not implemented
    bool alu = false;               // Run ALU-intensive kernel
    bool ssd = false;               // Run SSD I/O benchmark
    std::string ssd_path = "/tmp";  // SSD benchmark path
    std::size_t ssd_block_size = 4096; // SSD block size in bytes
    bool ssd_random = false;        // Random I/O vs sequential
    bool ssd_read_only = false;     // Read-only SSD benchmark
    bool run_apu = false;           // Run APU system identifier collection
    bool run_npu = false;           // Run NPU benchmark
    bool run_npu_suite = false;     // Run NPU benchmark suite
    bool run_medium_test = false;   // Run only medium test subset (default tests)
    bool quick_test = false;        // Run quick/short test
    bool show_platform = false;     // Display platform identification
    bool json_output = false;       // Output results in JSON format
    std::string json_filename = "benchmark_results.json"; // JSON output filename
    bool system_layout = false;     // Display system layout
    std::string layout_format = "text"; // Layout output format: text, mermaid, json
};

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " [options]\n"
              << "Options:\n"
              << "  -s, --size <MiB>       Size of each array (default: 256)\n"
              << "  -n, --iters <N>        Number of timed iterations per kernel (default: 20)\n"
              << "  -t, --type <float|double> Data type (default: float)\n"
              << "  -S, --simd             Enable SIMD (not implemented)\n"
              << "  -A, --alu              Run ALU-intensive kernel (multiply-add-multiply-add)\n"
              << "  -I, --ssd              Run SSD I/O benchmark\n"
              << "  --ssd-path <path>      SSD benchmark directory (default: /tmp)\n"
              << "  --ssd-block <size>     SSD block size in bytes (default: 4096)\n"
              << "  --ssd-random           Random I/O vs sequential\n"
              << "  --ssd-read-only        Read-only SSD benchmark\n"
               << "  -R, --run-apu          Run APU system identifier collection\n"
               << "  -P, --show-platform    Display platform identification\n"
               << "  -N, --run-npu          Run NPU benchmark\n"
               << "  --run-npu-suite        Run NPU benchmark suite (all precision/operation combinations)\n"
               << "  -M, --run-medium-test  Run only default test subset (excludes 1024 MiB stress test)\n"
               << "  -Q, --quick-test       Run quick/short test (smaller size, fewer iterations)\n"
               << "  -J, --json-output      Output results in JSON format\n"
               << "  --json-file <path>     JSON output filename (default: benchmark_results.json)\n"
               << "  -L, --system-layout    Display system layout (CPU/Memory/PCIe)\n"
               << "  --layout-format <fmt>  Layout format: text, mermaid, json (default: text)\n"
               << "  -h, --help             Show this help message\n";
}

bool parse_args(int argc, char* argv[], Options& opts) {
    std::vector<std::string> args(argv + 1, argv + argc);
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "-h" || a == "--help") { print_usage(argv[0]); return false; }
        else if (a == "-s" || a == "--size") {
            if (i + 1 >= args.size()) { std::cerr << "Missing value for " << a << "\n"; return false; }
            opts.sizeMiB = std::stoul(args[++i]);
        }
        else if (a == "-L" || a == "--system-layout") {
            opts.system_layout = true;
        }
        else if (a == "--layout-format") {
            if (i + 1 >= args.size()) { std::cerr << "Missing value for " << a << "\n"; return false; }
            opts.layout_format = args[++i];
            if (opts.layout_format != "text" && opts.layout_format != "mermaid" && opts.layout_format != "json") {
                std::cerr << "Invalid layout format: " << opts.layout_format << " (must be text, mermaid, or json)\n";
                return false;
            }
        }
        else if (a == "-n" || a == "--iters") {
            if (i + 1 >= args.size()) { std::cerr << "Missing value for " << a << "\n"; return false; }
            opts.iterations = std::stoul(args[++i]);
        }
        else if (a == "-t" || a == "--type") {
            if (i + 1 >= args.size()) { std::cerr << "Missing value for " << a << "\n"; return false; }
            opts.type = args[++i];
            if (opts.type != "float" && opts.type != "double") {
                std::cerr << "Invalid type: " << opts.type << " (must be float or double)\n";
                return false;
            }
        }
        else if (a == "-S" || a == "--simd") {
            opts.simd = true; // not used – placeholder for future SIMD implementations
        }
        else if (a == "-A" || a == "--alu") {
            opts.alu = true;
        }
        else if (a == "-I" || a == "--ssd") {
            opts.ssd = true;
        }
        else if (a == "--ssd-path") {
            if (i + 1 >= args.size()) { std::cerr << "Missing value for " << a << "\n"; return false; }
            opts.ssd_path = args[++i];
        }
        else if (a == "--ssd-block") {
            if (i + 1 >= args.size()) { std::cerr << "Missing value for " << a << "\n"; return false; }
            opts.ssd_block_size = std::stoul(args[++i]);
        }
        else if (a == "--ssd-random") {
            opts.ssd_random = true;
        }
        else if (a == "--ssd-read-only") {
            opts.ssd_read_only = true;
        }
        else if (a == "-R" || a == "--run-apu") {
            opts.run_apu = true;
        }
        else if (a == "-N" || a == "--run-npu") {
            opts.run_npu = true;
        }
        else if (a == "--run-npu-suite") {
            opts.run_npu_suite = true;
        }
        else if (a == "-M" || a == "--run-medium-test") {
            opts.run_medium_test = true;
        }
        else if (a == "-Q" || a == "--quick-test") {
            opts.quick_test = true;
        }
        else if (a == "-P" || a == "--show-platform") {
            opts.show_platform = true;
        }
        else {
            std::cerr << "Unknown option: " << a << "\n";
            return false;
        }
    }
    return true;
}

template <typename T>
void run_benchmark(const Options& opts) {
    const std::size_t element_size = sizeof(T);
    const std::size_t total_bytes = opts.sizeMiB * 1024 * 1024;
    const std::size_t n = total_bytes / element_size;

    double copy_time_total = 0.0;
    T* a = static_cast<T*>(mem_band::aligned_alloc(total_bytes, 64));
    T* b = static_cast<T*>(mem_band::aligned_alloc(total_bytes, 64));
    T* c = static_cast<T*>(mem_band::aligned_alloc(total_bytes, 64));
    if (!a || !b || !c) {
        std::cerr << "Allocation failed\n";
        std::exit(EXIT_FAILURE);
    }
    // Initialise arrays to avoid undefined behaviour
    for (std::size_t i = 0; i < n; ++i) {
        a[i] = static_cast<T>(1);
        b[i] = static_cast<T>(2);
        c[i] = static_cast<T>(0);
    }

    for (std::size_t iter = 0; iter < opts.iterations; ++iter) {
        auto start = std::chrono::high_resolution_clock::now();
    #ifdef SIMD_ENABLED
        if (opts.simd) {
            if constexpr (std::is_same_v<T, float>) {
                copy_kernel_simd(a, c, n);
            } else if constexpr (std::is_same_v<T, double>) {
                copy_kernel_simd(a, c, n);
            } else {
                copy_kernel<T>(a, c, n);
            }
        } else {
            copy_kernel<T>(a, c, n);
        }
    #else
        copy_kernel<T>(a, c, n);
    #endif
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        copy_time_total += diff.count();
    }
    double copy_time_avg = copy_time_total / static_cast<double>(opts.iterations);
    double copy_bytes = 2.0 * total_bytes; // read + write per element
    double copy_bandwidth = copy_bytes / copy_time_avg / 1e9; // GB/s

    // ---- Triad kernel ----
    double triad_time_total = 0.0;
    T scalar = static_cast<T>(3);
    for (std::size_t iter = 0; iter < opts.iterations; ++iter) {
        auto start = std::chrono::high_resolution_clock::now();
        if (opts.simd) {
            if constexpr (std::is_same_v<T, float>) {
                triad_kernel_simd(a, b, c, scalar, n);
            } else if constexpr (std::is_same_v<T, double>) {
                triad_kernel_simd(a, b, c, scalar, n);
            } else {
                triad_kernel<T>(a, b, c, scalar, n);
            }
        } else {
            triad_kernel<T>(a, b, c, scalar, n);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        triad_time_total += diff.count();
    }
    double triad_time_avg = triad_time_total / static_cast<double>(opts.iterations);
    double triad_bytes = 3.0 * total_bytes; // two reads + one write
    double triad_bandwidth = triad_bytes / triad_time_avg / 1e9; // GB/s

    // ---- Random read/write kernel ----
    double rand_time_total = 0.0;
    for (std::size_t iter = 0; iter < opts.iterations; ++iter) {
        auto start = std::chrono::high_resolution_clock::now();
        random_rw_kernel<T>(a, c, n);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        rand_time_total += diff.count();
    }
    double rand_time_avg = rand_time_total / static_cast<double>(opts.iterations);
    double rand_bytes = 2.0 * total_bytes; // read + write per element (random access)
    double rand_bandwidth = rand_bytes / rand_time_avg / 1e9; // GB/s

    // ---- ALU kernel (if requested) --
    double alu_time_total = 0.0;
    if (opts.alu) {
        for (std::size_t iter = 0; iter < opts.iterations; ++iter) {
            auto start = std::chrono::high_resolution_clock::now();
            alu_kernel(a, b, c, n);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = end - start;
            alu_time_total += diff.count();
        }
        double alu_time_avg = alu_time_total / static_cast<double>(opts.iterations);
        // ALU operations: 2 multiplies, 2 adds per element = 4 FLOPS per element
        // Bytes accessed: 3 reads (a, b, c) + 1 write (a) = 4 * element_size per element
        double alu_bytes = 4.0 * total_bytes; // 3 reads + 1 write per element
        double alu_bandwidth = alu_bytes / alu_time_avg / 1e9; // GB/s

        // Output results
        std::cout << "# Size: " << opts.sizeMiB << " MiB, Type: " << opts.type
                  << ", Iterations: " << opts.iterations << "\n";
        std::cout << "Kernel   Bytes/Iter  Time(s)   Bandwidth(GB/s)\n";
        std::cout << "Copy     " << static_cast<std::size_t>(copy_bytes) << "    "
                  << copy_time_avg << "    " << copy_bandwidth << "\n";
        std::cout << "Triad    " << static_cast<std::size_t>(triad_bytes) << "    "
                  << triad_time_avg << "    " << triad_bandwidth << "\n";
        std::cout << "RandomRW " << static_cast<std::size_t>(rand_bytes) << "    "
                  << rand_time_avg << "    " << rand_bandwidth << "\n";
        std::cout << "ALU      " << static_cast<std::size_t>(alu_bytes) << "    "
                  << alu_time_avg << "    " << alu_bandwidth << "\n";

        aligned_free(a);
        aligned_free(b);
        aligned_free(c);
        return;
    }

    // Output results (original format if ALU not requested)
    std::cout << "# Size: " << opts.sizeMiB << " MiB, Type: " << opts.type
              << ", Iterations: " << opts.iterations << "\n";
    std::cout << "Kernel   Bytes/Iter  Time(s)   Bandwidth(GB/s)\n";
    std::cout << "Copy     " << static_cast<std::size_t>(copy_bytes) << "    "
              << copy_time_avg << "    " << copy_bandwidth << "\n";
    std::cout << "Triad    " << static_cast<std::size_t>(triad_bytes) << "    "
              << triad_time_avg << "    " << triad_bandwidth << "\n";
    std::cout << "RandomRW " << static_cast<std::size_t>(rand_bytes) << "    "
              << rand_time_avg << "    " << rand_bandwidth << "\n";

    aligned_free(a);
    aligned_free(b);
    aligned_free(c);
}

void run_platform_detection() {
    auto info = mem_band::PlatformDetection::detect();
    auto cpu_vendor = mem_band::PlatformDetection::get_cpu_vendor();
    auto cpu_isa = mem_band::PlatformDetection::get_cpu_isa();
    
    std::cout << "# Platform Identification\n";
    std::cout << "System: " << info.cpu_vendor << " " << cpu_isa << "\n";
    std::cout << "Detected devices:\n";
    for (const auto& device : info.pci_devices) {
        std::cout << "  " << device.vendor << " " << device.device << " (Class: " << device.class_info << ")\n";
    }
}

// Run SSD benchmark
void run_ssd_benchmark(const Options& opts) {
    mem_band::SSDConfig config{};
    config.path = opts.ssd_path;
    config.block_size = opts.ssd_block_size;
    config.num_blocks = 100; // Fixed for now
    config.sequential = !opts.ssd_random;
    config.read_only = opts.ssd_read_only;
    
    std::cout << "# SSD I/O Benchmark\n";
    std::cout << "# Path: " << opts.ssd_path << ", Block Size: " << opts.ssd_block_size 
              << " bytes, Random I/O: " << (opts.ssd_random ? "yes" : "no") << "\n\n";
    
    auto result = mem_band::run_ssd_benchmark(config);
    
    std::cout << "Benchmark     Bandwidth(MB/s)    IOPS      Latency(us)\n";
    
    std::string iotype = opts.ssd_random ? (opts.ssd_read_only ? "RandomRead" : "RandomWrite") 
                                          : (opts.ssd_read_only ? "SequentialRead" : "SequentialWrite");
    std::cout << iotype << "     " << result.bandwidth_mbps << "    " << result.iops << "    " << result.latency_us << "\n";
}

// Run APU benchmark (system identifier)
void run_apu_benchmark(const Options& opts) {
    mem_band::APUSystemInfo info = mem_band::collect_apu_system_info();
    mem_band::print_system_info(info);
}

// Run NPU benchmark
void run_npu_benchmark(const Options& opts) {
    mem_band::NPUConfig config;
    
    if (opts.run_npu_suite) {
        std::cout << "# Running NPU benchmark suite\n\n";
        auto results = mem_band::run_npu_benchmark_suite(config);
        for (const auto& result : results) {
            mem_band::print_npu_result(result, config);
            std::cout << "\n";
        }
    } else {
        std::cout << "# NPU Benchmark\n\n";
        auto result = mem_band::mock_npu_benchmark(config);
        mem_band::print_npu_result(result, config);
    }
}

void run_layout(const Options& opts) {
    auto info = mem_band::PlatformDetection::detect();
    auto cpu_vendor = mem_band::PlatformDetection::get_cpu_vendor();
    auto cpu_isa = mem_band::PlatformDetection::get_cpu_isa();
    
    LayoutBuilder builder;
    
    builder.add_cpu(info.cpu_vendor + " " + cpu_isa, 8, 16);
    
    auto sys_info = SystemInfo::collect();
    builder.add_memory(sys_info.memory_size_mb, 2);
    
    for (const auto& device : info.pci_devices) {
        std::string device_type = "Unknown";
        if (device.class_info.find("0300") != std::string::npos || 
            device.class_info.find("0380") != std::string::npos) {
            device_type = "GPU";
        } else if (device.vendor == "1002") {
            device_type = "NPU";
        }
        builder.add_pci_device(device.vendor, device.device, device_type, 16);
    }
    
    auto layout = builder.build();
    
    std::unique_ptr<LayoutFormatter> formatter;
    if (opts.layout_format == "text") {
        formatter = std::make_unique<TextFormatter>();
    } else if (opts.layout_format == "mermaid") {
        formatter = std::make_unique<MermaidFormatter>();
    } else if (opts.layout_format == "json") {
        formatter = std::make_unique<JSONFormatter>();
    }
    
    std::cout << "# System Layout (" << opts.layout_format << " format)\n";
    std::cout << formatter->format(layout);
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    Options opts;
    if (argc == 1) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    if (!parse_args(argc, argv, opts)) {
        return EXIT_FAILURE;
    }

    // Quick test mode: reduce size and iterations
    if (opts.quick_test) {
        opts.sizeMiB = 128;
        opts.iterations = 10;
    }

    // Show system layout
    if (opts.system_layout) {
        run_layout(opts);
        return EXIT_SUCCESS;
    }
    
    // Show platform identification
    if (opts.show_platform) {
        run_platform_detection();
        return EXIT_SUCCESS;
    }

    // Run APU system identifier
    if (opts.run_apu) {
        run_apu_benchmark(opts);
        return EXIT_SUCCESS;
    }

    // Run NPU benchmark
    if (opts.run_npu || opts.run_npu_suite) {
        run_npu_benchmark(opts);
        return EXIT_SUCCESS;
    }

    // Run SSD benchmark
    if (opts.ssd) {
        run_ssd_benchmark(opts);
        return EXIT_SUCCESS;
    }

    // Run medium test subset (modify iterations for quick test)
    if (opts.run_medium_test) {
        std::cout << "# Running medium test subset (excludes 1024 MiB stress test)\n";
        if (opts.type == "float") {
            run_benchmark<float>(opts);
        } else {
            run_benchmark<double>(opts);
        }
        return EXIT_SUCCESS;
    }

    // Default: run memory bandwidth benchmark
    if (opts.type == "float") {
        run_benchmark<float>(opts);
    } else {
        run_benchmark<double>(opts);
    }

    // Show platform identification after memory run (if not explicitly requested)
    if (!opts.quick_test || !opts.show_platform) {
        run_platform_detection();
    }
    
    // Report available runtime features with warnings for unavailable ones
    {
        RuntimeDetector detector;
        detector.register_feature("CUDA Runtime", runtime::check_cuda_runtime, "NVIDIA CUDA runtime library available for GPU computing");
        detector.register_feature("ROCm Runtime", runtime::check_rocm_runtime, "AMD ROCm runtime library available for GPU computing");
        detector.register_feature("JSON Output", runtime::check_json_output, "nlohmann/json library available for JSON serialization");
        detector.register_feature("SQLite Output", runtime::check_sqlite_output, "SQLite3 library available for persistent storage");
        
        auto available = detector.get_available_features();
        auto unavailable = detector.get_unavailable_features();
        
        std::cout << "\n# Runtime Features\n";
        std::cout << "Available features:\n";
        for (const auto& f : available) {
            std::cout << "  ✓ " << f.name << " - " << f.description << "\n";
        }
        
        if (!unavailable.empty()) {
            std::cout << "\nUnavailable optional features (run with feature flags to enable):\n";
            for (const auto& f : unavailable) {
                std::cout << "  ✗ " << f.name << " - " << f.description << "\n";
            }
        }
    }

    return EXIT_SUCCESS;
}
