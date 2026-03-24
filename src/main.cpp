// main.cpp
// Command‑line interface for the memory bandwidth benchmark.
// Parses options, allocates aligned arrays, runs copy and triad kernels,
// measures execution time, and prints CSV‑friendly results.

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <type_traits>

#include "aligned_alloc.hpp"
#include "benchmark.hpp"

using namespace mem_band;

struct Options {
    std::size_t sizeMiB = 256;      // array size in MiB
    std::size_t iterations = 20;    // timed iterations per kernel
    std::string type = "float";   // data type: float or double
    bool simd = false;              // placeholder – not implemented
};

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " [options]\n"
              << "Options:\n"
              << "  -s, --size <MiB>       Size of each array (default: 256)\n"
              << "  -n, --iters <N>        Number of timed iterations per kernel (default: 20)\n"
              << "  -t, --type <float|double> Data type (default: float)\n"
              << "  -S, --simd             Enable SIMD (not implemented)\n"
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

    // Output results
    std::cout << "# Size: " << opts.sizeMiB << " MiB, Type: " << opts.type
              << ", Iterations: " << opts.iterations << "\n";
    std::cout << "Kernel   Bytes/Iter  Time(s)   Bandwidth(GB/s)\n";
    std::cout << "Copy     " << static_cast<std::size_t>(copy_bytes) << "    "
              << copy_time_avg << "    " << copy_bandwidth << "\n";
    std::cout << "Triad    " << static_cast<std::size_t>(triad_bytes) << "    "
              << triad_time_avg << "    " << triad_bandwidth << "\n";

    aligned_free(a);
    aligned_free(b);
    aligned_free(c);
}

int main(int argc, char* argv[]) {
    Options opts;
    if (!parse_args(argc, argv, opts)) {
        return EXIT_FAILURE;
    }

    if (opts.type == "float") {
        run_benchmark<float>(opts);
    } else {
        run_benchmark<double>(opts);
    }
    return EXIT_SUCCESS;
}
