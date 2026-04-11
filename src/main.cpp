// main.cpp
// Command-line interface for the memory bandwidth benchmark.
// Parses options, allocates aligned arrays, runs copy/triad/random/ALU kernels,
// measures execution time, and prints CSV-friendly results.

#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <chrono>
#include <functional>
#include <iomanip>
#include <cstdio>

#include "aligned_alloc.hpp"
#include "benchmark.hpp"
#include "ssd_benchmark.hpp"
#include "apu_identifier.hpp"
#include "npu_benchmark.hpp"
#include "platform_detection.hpp"
#include "system_info.hpp"
#include "runtime_detection.hpp"
#include "json_output.hpp"
#include "sqlite_output.hpp"
#include "sqlite_input.hpp"
#include "layout_builder.hpp"

using namespace mem_band;

// ---------------------------------------------------------------------------
// Options
// ---------------------------------------------------------------------------

struct Options {
    std::string benchmark = "memory";
    std::string info_cmd;

    std::size_t size_mib = 256;
    std::size_t iterations = 20;
    std::string type = "float";
    bool simd = false;
    bool alu = false;

    std::string ssd_path = "/tmp";
    std::size_t ssd_block_size = 4096;
    bool ssd_random = false;
    bool ssd_read_only = false;

    bool quick_test = false;
    bool medium_test = false;

    bool show_platform = false;
    bool show_runtime_features = false;
    bool system_layout = false;
    std::string layout_format = "text";

    std::string output_format = "text";
    std::string output_file;
    
#ifdef ENABLE_SQLITE
    std::string db_path = "~/.mem_band/benchmarks.db";
    bool list_benchmarks = false;
    bool search_benchmarks = false;
    std::string search_pattern;
    bool export_db = false;
    std::string export_format;
    std::string export_file;
#endif
};

#ifdef ENABLE_SQLITE
void run_sqlite_browsing(const Options& opts);
#endif

// ---------------------------------------------------------------------------
// Usage
// ---------------------------------------------------------------------------

void print_usage(const char* prog) {
    std::cout
        << "Usage: " << prog << " <benchmark> [options]\n"
        << "\n"
        << "Benchmarks:\n"
        << "  memory                   Memory bandwidth benchmarks (default)\n"
        << "  ssd                      SSD I/O benchmark\n"
        << "  apu                      APU system identifier\n"
        << "  npu                      NPU benchmark\n"
        << "  npu-suite                NPU benchmark suite\n"
        << "  info                     System information subcommands\n"
        << "\n"
        << "Examples:\n"
        << "  " << prog << " memory --size 512 --iters 10\n"
        << "  " << prog << " memory --quick-test\n"
        << "  " << prog << " ssd --ssd-path /tmp --ssd-block 4096\n"
        << "  " << prog << " info platform\n"
        << "  " << prog << " info layout\n"
        << "\n"
        << "Memory benchmark options:\n"
        << "  -s, --size <MiB>           Array size in MiB (default: 256)\n"
        << "  -n, --iters <N>            Iterations per kernel (default: 20)\n"
        << "  -t, --type <float|double>  Data type (default: float)\n"
        << "  -S, --simd                 Enable SIMD kernels (requires build flag)\n"
        << "  -A, --alu                  Include ALU-intensive kernel\n"
        << "  -M, --run-medium-test      Medium test (256 MiB, standard iterations)\n"
        << "  -Q, --quick-test           Quick test (64 MiB, 5 iterations)\n"
        << "\n"
        << "SSD benchmark options:\n"
        << "  --ssd-path <path>          Benchmark directory (default: /tmp)\n"
        << "  --ssd-block <bytes>        Block size (default: 4096)\n"
        << "  --ssd-random               Use random I/O (default: sequential)\n"
        << "  --ssd-read-only            Read-only benchmark\n"
        << "\n"
        << "System information:\n"
        << "  info platform              Show platform identification\n"
        << "  info features              Show available runtime features\n"
        << "  info layout                Show system layout diagram\n"
        << "  info layout --format <fmt> Layout format: text, mermaid, json\n"
        << "\n"
        << "Output options (memory benchmark only):\n"
        << "  -o, --output-format <fmt>  Output format: text, csv, json (default: text)\n"
        << "  -f, --output-file <path>   Write results to file (default: stdout)\n"
        
#ifdef ENABLE_SQLITE
        << "\n"
        << "Database commands (SQLite):\n"
        << "  --db-path <path>           Database path (default: ~/.mem_band/benchmarks.db)\n"
        << "  --list-benchmarks          List all benchmark runs in database\n"
        << "  --search <pattern>         Search benchmark results by pattern\n"
        << "  --export-db <fmt> <file>   Export database to CSV/JSON file\n"
#endif
        << "  -h, --help                 Show this help message\n";
}

// ---------------------------------------------------------------------------
// Argument parser
// ---------------------------------------------------------------------------

bool parse_args(int argc, char* argv[], Options& opts) {
    std::vector<std::string> args(argv + 1, argv + argc);
    
    if (args.empty()) {
        print_usage(argv[0]);
        return false;
    }

    std::string first = args[0];
    
    if (first == "-h" || first == "--help") {
        print_usage(argv[0]);
        return false;
    }
    
    if (first == "memory") {
        opts.benchmark = "memory";
        args.erase(args.begin());
    } else if (first == "ssd") {
        opts.benchmark = "ssd";
        args.erase(args.begin());
    } else if (first == "apu") {
        opts.benchmark = "apu";
        args.erase(args.begin());
    } else if (first == "npu") {
        opts.benchmark = "npu";
        args.erase(args.begin());
    } else if (first == "npu-suite") {
        opts.benchmark = "npu-suite";
        args.erase(args.begin());
    } else if (first == "info") {
        opts.benchmark = "info";
        args.erase(args.begin());
        if (args.empty()) {
            std::cerr << "Missing info subcommand. Use 'info platform', 'info features', or 'info layout'\n";
            return false;
        }
        opts.info_cmd = args[0];
        args.erase(args.begin());
    } else {
        opts.benchmark = first;
        args.erase(args.begin());
    }

    auto require_value = [&](const std::string& flag) -> bool {
        if (args.empty()) {
            std::cerr << "Missing value for " << flag << "\n";
            return false;
        }
        return true;
    };
    
    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];

        auto advance = [&]() -> std::string {
            if (i + 1 >= args.size()) {
                return "";
            }
            i++;
            return args[i];
        };

        if (a == "-s" || a == "--size") {
            std::string val = advance();
            if (val.empty()) {
                std::cerr << "Missing value for " << a << "\n";
                return false;
            }
            opts.size_mib = std::stoul(val);
        }
        else if (a == "-n" || a == "--iters") {
            std::string val = advance();
            if (val.empty()) {
                std::cerr << "Missing value for " << a << "\n";
                return false;
            }
            opts.iterations = std::stoul(val);
        }
        else if (a == "-t" || a == "--type") {
            std::string val = advance();
            if (val.empty()) {
                std::cerr << "Missing value for " << a << "\n";
                return false;
            }
            opts.type = val;
            if (opts.type != "float" && opts.type != "double") {
                std::cerr << "Invalid type: " << opts.type << " (must be float or double)\n";
                return false;
            }
        }
        else if (a == "-S" || a == "--simd") { opts.simd = true; }
        else if (a == "-A" || a == "--alu")  { opts.alu = true; }
        else if (a == "--ssd-path") {
            std::string val = advance();
            if (val.empty()) {
                std::cerr << "Missing value for " << a << "\n";
                return false;
            }
            opts.ssd_path = val;
        }
        else if (a == "--ssd-block") {
            std::string val = advance();
            if (val.empty()) {
                std::cerr << "Missing value for " << a << "\n";
                return false;
            }
            opts.ssd_block_size = std::stoul(val);
        }
        else if (a == "--ssd-random")    { opts.ssd_random = true; }
        else if (a == "--ssd-read-only") { opts.ssd_read_only = true; }
        else if (a == "--layout-format") {
            std::string val = advance();
            if (val.empty()) {
                std::cerr << "Missing value for " << a << "\n";
                return false;
            }
            opts.layout_format = val;
            if (opts.layout_format != "text" && opts.layout_format != "mermaid" && opts.layout_format != "json") {
                std::cerr << "Invalid layout format: " << opts.layout_format
                          << " (must be text, mermaid, or json)\n";
                return false;
            }
        }
        else if (a == "-M" || a == "--run-medium-test") { opts.medium_test = true; }
        else if (a == "-Q" || a == "--quick-test")      { opts.quick_test = true; }
        else if (a == "-o" || a == "--output-format") {
            std::string val = advance();
            if (val.empty()) {
                std::cerr << "Missing value for " << a << "\n";
                return false;
            }
            opts.output_format = val;
            if (opts.output_format != "text" && opts.output_format != "csv" && opts.output_format != "json") {
                std::cerr << "Invalid output format: " << opts.output_format
                          << " (must be text, csv, or json)\n";
                return false;
            }
        }
        else if (a == "-f" || a == "--output-file") {
            std::string val = advance();
            if (val.empty()) {
                std::cerr << "Missing value for " << a << "\n";
                return false;
            }
            opts.output_file = val;
        }
#ifdef ENABLE_SQLITE
        else if (a == "--db-path") {
            std::string val = advance();
            if (val.empty()) {
                std::cerr << "Missing value for " << a << "\n";
                return false;
            }
            opts.db_path = val;
        }
        else if (a == "--list-benchmarks") { opts.list_benchmarks = true; }
        else if (a == "--search") {
            std::string val = advance();
            if (val.empty()) {
                std::cerr << "Missing value for " << a << "\n";
                return false;
            }
            opts.search_benchmarks = true;
            opts.search_pattern = val;
        }
        else if (a == "--export-db") {
            std::string val1 = advance();
            std::string val2 = advance();
            if (val1.empty() || val2.empty()) {
                std::cerr << "Missing values for " << a << "\n";
                return false;
            }
            opts.export_db = true;
            opts.export_format = val1;
            opts.export_file = val2;
        }
#endif
        else {
            std::cerr << "Unknown option: " << a << "\n";
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// Benchmark helpers
// ---------------------------------------------------------------------------

// Time a kernel over `iterations` runs; returns average wall-clock seconds.
template <typename Func>
double time_kernel(Func&& fn, std::size_t iterations) {
    double total = 0.0;
    for (std::size_t iter = 0; iter < iterations; ++iter) {
        auto t0 = std::chrono::high_resolution_clock::now();
        fn();
        auto t1 = std::chrono::high_resolution_clock::now();
        total += std::chrono::duration<double>(t1 - t0).count();
    }
    return total / static_cast<double>(iterations);
}

struct KernelResult {
    std::string name;
    double bytes_per_iter;
    double avg_time_s;
    double bandwidth_gbs;
};

template <typename T>
std::vector<KernelResult> run_benchmark(const Options& opts) {
    const std::size_t total_bytes = opts.size_mib * 1024 * 1024;
    const std::size_t n = total_bytes / sizeof(T);

    T* a = static_cast<T*>(mem_band::aligned_alloc(total_bytes, 64));
    T* b = static_cast<T*>(mem_band::aligned_alloc(total_bytes, 64));
    T* c = static_cast<T*>(mem_band::aligned_alloc(total_bytes, 64));
    if (!a || !b || !c) {
        std::cerr << "Allocation failed\n";
        std::exit(EXIT_FAILURE);
    }

    // Initialise to avoid UB and ensure first-touch page placement.
    for (std::size_t i = 0; i < n; ++i) {
        a[i] = static_cast<T>(1);
        b[i] = static_cast<T>(2);
        c[i] = static_cast<T>(0);
    }

    std::vector<KernelResult> results;

    // ---- Copy ----
    {
        auto avg = time_kernel([&]() {
        #ifdef SIMD_ENABLED
            if (opts.simd) {
                copy_kernel_simd(a, c, n);
            } else {
                copy_kernel<T>(a, c, n);
            }
        #else
            copy_kernel<T>(a, c, n);
        #endif
        }, opts.iterations);

        double bytes = 2.0 * total_bytes;
        results.push_back({"Copy", bytes, avg, bytes / avg / 1e9});
    }

    // ---- Triad ----
    {
        T scalar = static_cast<T>(3);
        auto avg = time_kernel([&]() {
            if (opts.simd) {
                triad_kernel_simd(a, b, c, scalar, n);
            } else {
                triad_kernel<T>(a, b, c, scalar, n);
            }
        }, opts.iterations);

        double bytes = 3.0 * total_bytes;
        results.push_back({"Triad", bytes, avg, bytes / avg / 1e9});
    }

    // ---- RandomRW ----
    {
        auto avg = time_kernel([&]() {
            random_rw_kernel<T>(a, c, n);
        }, opts.iterations);

        double bytes = 2.0 * total_bytes;
        results.push_back({"RandomRW", bytes, avg, bytes / avg / 1e9});
    }

    // ---- ALU (optional) ----
    if (opts.alu) {
        // Re-init a so values are reasonable for ALU
        for (std::size_t i = 0; i < n; ++i) a[i] = static_cast<T>(1);

        auto avg = time_kernel([&]() {
            alu_kernel(a, b, c, n);
        }, opts.iterations);

        double bytes = 4.0 * total_bytes;  // 3 reads + 1 write
        results.push_back({"ALU", bytes, avg, bytes / avg / 1e9});
    }

    // ---- Print results ----
    std::cout << "# Size: " << opts.size_mib << " MiB, Type: " << opts.type
              << ", Iterations: " << opts.iterations << "\n";
    std::cout << std::left << std::setw(10) << "Kernel"
              << std::right << std::setw(14) << "Data/Iter"
              << std::setw(14) << "Time(ms)"
              << std::setw(18) << "Bandwidth(GB/s)" << "\n";

    for (const auto& r : results) {
        double data_mib = r.bytes_per_iter / (1024.0 * 1024.0);
        double time_ms = r.avg_time_s * 1000.0;

        // Format data size with appropriate unit
        std::string data_str;
        if (data_mib >= 1024.0) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%.1f GiB", data_mib / 1024.0);
            data_str = buf;
        } else {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%.0f MiB", data_mib);
            data_str = buf;
        }

        std::cout << std::left  << std::setw(10) << r.name
                  << std::right << std::setw(14) << data_str
                  << std::setw(14) << std::fixed << std::setprecision(2) << time_ms
                  << std::setw(18) << std::setprecision(2) << r.bandwidth_gbs << "\n";
    }

    aligned_free(a);
    aligned_free(b);
    aligned_free(c);
    
    return results;
}

#ifdef ENABLE_SQLITE
// -----------------------------------------------------------------------------
// SQLite output helpers
// -----------------------------------------------------------------------------
void run_sqlite_output(const Options& opts, const std::vector<KernelResult>& results, 
                       const std::string& system_id) {
    SQLiteOutput sqlite(opts.db_path);
    
    // Convert time_kernel output to get actual bytes_per_iter and bandwidth
    // Results are already in results vector from run_benchmark
    for (const auto& r : results) {
        BenchmarkResult br;
        br.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        br.system_id = system_id;
        br.kernel = r.name;
        br.size_mib = opts.size_mib;
        br.data_type = opts.type;
        br.iterations = opts.iterations;
        br.bandwidth_gb_s = r.bandwidth_gbs;
        br.time_seconds = r.avg_time_s;
        br.bytes_per_iter = static_cast<unsigned long long>(r.bytes_per_iter);
        br.cpu_model = "";
        br.os_name = "";
        br.os_version = "";
        
        sqlite.append(br);
    }
}
#endif

// ---------------------------------------------------------------------------
// Platform / system info helpers
// ---------------------------------------------------------------------------

void run_platform_detection() {
    auto info = mem_band::PlatformDetection::detect();
    auto cpu_isa = mem_band::PlatformDetection::get_cpu_isa();

    std::cout << "# Platform Identification\n";
    std::cout << "System: " << info.cpu_vendor << " " << cpu_isa << "\n";
    if (!info.pci_devices.empty()) {
        std::cout << "Detected devices:\n";
        for (const auto& device : info.pci_devices) {
            std::cout << "  " << device.vendor << " " << device.device
                      << " (Class: " << device.class_info << ")\n";
        }
    }
}

void print_runtime_features() {
    RuntimeDetector detector;
    detector.register_feature("CUDA Runtime", runtime::check_cuda_runtime,
                              "NVIDIA CUDA runtime library available for GPU computing");
    detector.register_feature("ROCm Runtime", runtime::check_rocm_runtime,
                              "AMD ROCm runtime library available for GPU computing");
    detector.register_feature("JSON Output", runtime::check_json_output,
                              "nlohmann/json library available for JSON serialization");
    detector.register_feature("SQLite Output", runtime::check_sqlite_output,
                              "SQLite3 library available for persistent storage");

    auto available   = detector.get_available_features();
    auto unavailable = detector.get_unavailable_features();

    std::cout << "\n# Runtime Features\n";
    std::cout << "Available features:\n";
    for (const auto& f : available) {
        std::cout << "  + " << f.name << " - " << f.description << "\n";
    }
    if (!unavailable.empty()) {
        std::cout << "\nUnavailable optional features:\n";
        for (const auto& f : unavailable) {
            std::cout << "  - " << f.name << " - " << f.description << "\n";
        }
    }
}

// ---------------------------------------------------------------------------
// SSD benchmark
// ---------------------------------------------------------------------------

void run_ssd_benchmark(const Options& opts) {
    mem_band::SSDConfig config{};
    config.path = opts.ssd_path;
    config.block_size = opts.ssd_block_size;
    config.num_blocks = 100;
    config.sequential = !opts.ssd_random;
    config.read_only = opts.ssd_read_only;

    std::cout << "# SSD I/O Benchmark\n";
    std::cout << "# Path: " << opts.ssd_path
              << ", Block Size: " << opts.ssd_block_size
              << " bytes, Random I/O: " << (opts.ssd_random ? "yes" : "no") << "\n\n";

    auto result = mem_band::run_ssd_benchmark(config);

    std::string iotype = opts.ssd_random
        ? (opts.ssd_read_only ? "RandomRead" : "RandomWrite")
        : (opts.ssd_read_only ? "SequentialRead" : "SequentialWrite");

    // Format bandwidth with appropriate unit
    std::string bw_str;
    if (result.bandwidth_mbps >= 1024.0) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.2f GB/s", result.bandwidth_mbps / 1024.0);
        bw_str = buf;
    } else {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.2f MB/s", result.bandwidth_mbps);
        bw_str = buf;
    }

    // Format IOPS with K/M suffix
    std::string iops_str;
    if (result.iops >= 1e6) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.2f M", result.iops / 1e6);
        iops_str = buf;
    } else if (result.iops >= 1e3) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.2f K", result.iops / 1e3);
        iops_str = buf;
    } else {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.0f", result.iops);
        iops_str = buf;
    }

    std::cout << std::left  << std::setw(18) << "Benchmark"
              << std::right << std::setw(16) << "Bandwidth"
              << std::setw(14) << "IOPS"
              << std::setw(14) << "Latency(us)" << "\n";
    std::cout << std::left  << std::setw(18) << iotype
              << std::right << std::setw(16) << bw_str
              << std::setw(14) << iops_str
              << std::setw(14) << std::fixed << std::setprecision(2) << result.latency_us << "\n";
}

// ---------------------------------------------------------------------------
// APU / NPU
// ---------------------------------------------------------------------------

void run_apu_benchmark() {
    mem_band::APUSystemInfo info = mem_band::collect_apu_system_info();
    mem_band::print_system_info(info);
}

void run_npu_benchmark(const Options& opts) {
    mem_band::NPUConfig config;

    if (opts.benchmark == "npu-suite") {
        std::cout << "# Running NPU benchmark suite\n\n";
        auto results = mem_band::run_npu_benchmark_suite(config);
        mem_band::print_npu_suite_results(results);
    } else {
        auto result = mem_band::mock_npu_benchmark(config);
        mem_band::print_npu_result(result, config);
    }
}

// ---------------------------------------------------------------------------
// System layout
// ---------------------------------------------------------------------------

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
    if (opts.layout_format == "mermaid") {
        formatter = std::make_unique<MermaidFormatter>();
    } else if (opts.layout_format == "json") {
        formatter = std::make_unique<JSONFormatter>();
    } else {
        formatter = std::make_unique<TextFormatter>();
    }

    std::cout << "# System Layout (" << opts.layout_format << " format)\n";
    std::cout << formatter->format(layout);
    std::cout << "\n";
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    Options opts;

    if (argc == 1) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    if (!parse_args(argc, argv, opts)) {
        return EXIT_FAILURE;
    }

    if (opts.quick_test) {
        opts.size_mib = 64;
        opts.iterations = 5;
    }

    if (opts.info_cmd == "platform") {
        run_platform_detection();
        return EXIT_SUCCESS;
    }

    if (opts.info_cmd == "features") {
        print_runtime_features();
        return EXIT_SUCCESS;
    }

    if (opts.info_cmd == "layout") {
        run_layout(opts);
        return EXIT_SUCCESS;
    }

    if (opts.benchmark == "apu") {
        run_apu_benchmark();
        return EXIT_SUCCESS;
    }

    if (opts.benchmark == "npu" || opts.benchmark == "npu-suite") {
        run_npu_benchmark(opts);
        return EXIT_SUCCESS;
    }

#ifdef ENABLE_SQLITE
    if (opts.list_benchmarks || opts.search_benchmarks || opts.export_db) {
        run_sqlite_browsing(opts);
        return EXIT_SUCCESS;
    }
#endif

    if (opts.benchmark == "ssd") {
        run_ssd_benchmark(opts);
        return EXIT_SUCCESS;
    }

    if (opts.medium_test) {
        std::cout << "# Running medium test subset (excludes 1024 MiB stress test)\n";
    }

    if (opts.type == "float") {
        auto results = run_benchmark<float>(opts);
#ifdef ENABLE_SQLITE
        auto sys_info = mem_band::SystemInfo::collect();
        std::string system_id = mem_band::SystemInfo::generate_hash(sys_info);
        run_sqlite_output(opts, results, system_id);
#endif
    } else {
        auto results = run_benchmark<double>(opts);
#ifdef ENABLE_SQLITE
        auto sys_info = mem_band::SystemInfo::collect();
        std::string system_id = mem_band::SystemInfo::generate_hash(sys_info);
        run_sqlite_output(opts, results, system_id);
#endif
    }

    return EXIT_SUCCESS;
}

#ifdef ENABLE_SQLITE
// -----------------------------------------------------------------------------
// SQLite database browsing
// -----------------------------------------------------------------------------

void run_sqlite_browsing(const Options& opts) {
    SQLiteInput input(opts.db_path);
    std::cout << "# Database: " << opts.db_path << "\n";
    
    if (opts.list_benchmarks) {
        std::cout << "# Listing benchmarks\n";
        std::cout << std::left << std::setw(6) << "ID"
                  << std::setw(20) << "Timestamp"
                  << std::setw(14) << "System ID"
                  << std::setw(12) << "Kernel"
                  << std::setw(10) << "Size(MiB)"
                  << std::setw(8) << "Type"
                  << std::setw(16) << "Bandwidth(GB/s)" << "\n";
        std::cout << std::string(88, '-') << "\n";
        
        auto results = input.query_all();
        if (results.empty()) {
            std::cout << "No benchmarks found in database.\n";
        } else {
            for (size_t i = 0; i < results.size(); ++i) {
                const auto& r = results[i];
                std::cout << std::left << std::setw(6) << (i + 1)
                          << std::setw(20) << r.timestamp
                          << std::setw(14) << r.system_id.substr(0, 12)
                          << std::setw(12) << r.kernel
                          << std::setw(10) << r.size_mib
                          << std::setw(8) << r.data_type
                          << std::setw(16) << std::fixed << std::setprecision(2) << r.bandwidth_gb_s << "\n";
            }
        }
    }
    
    if (opts.search_benchmarks) {
        std::cout << "# Searching: " << opts.search_pattern << "\n";
        std::cout << std::left << std::setw(6) << "ID"
                  << std::setw(20) << "Timestamp"
                  << std::setw(14) << "System ID"
                  << std::setw(12) << "Kernel"
                  << std::setw(10) << "Size(MiB)"
                  << std::setw(8) << "Type"
                  << std::setw(16) << "Bandwidth(GB/s)" << "\n";
        std::cout << std::string(88, '-') << "\n";
        
        auto results = input.search(opts.search_pattern);
        if (results.empty()) {
            std::cout << "No matching benchmarks found.\n";
        } else {
            for (size_t i = 0; i < results.size(); ++i) {
                const auto& r = results[i];
                std::cout << std::left << std::setw(6) << (i + 1)
                          << std::setw(20) << r.timestamp
                          << std::setw(14) << r.system_id.substr(0, 12)
                          << std::setw(12) << r.kernel
                          << std::setw(10) << r.size_mib
                          << std::setw(8) << r.data_type
                          << std::setw(16) << std::fixed << std::setprecision(2) << r.bandwidth_gb_s << "\n";
            }
        }
    }
    
    if (opts.export_db) {
        std::cout << "# Exporting to " << opts.export_format << ": " << opts.export_file << "\n";
        
        bool success = false;
        if (opts.export_format == "csv") {
            success = input.export_csv(opts.export_file);
        } else if (opts.export_format == "json") {
            success = input.export_json(opts.export_file);
        } else {
            std::cerr << "Invalid export format: " << opts.export_format
                      << " (must be csv or json)\n";
            return;
        }
        
        if (success) {
            std::cout << "Export successful.\n";
        } else {
            std::cerr << "Export failed.\n";
        }
    }
}
#endif
