# Memory Bandwidth Benchmark

![Generated with local AI](https://img.shields.io/badge/Generated%20with%20local%20AI-blue) ![Hardware](https://img.shields.io/badge/Hardware-NVIDIA%20DGX%20Spark%20GB10-blue) ![Model](https://img.shields.io/badge/Model-nemotron--3--super%3A120b-blue) ![CI Build Status](https://img.shields.io/github/actions/workflow/status/phaus/memory-vibes/ci.yml?branch=main&logo=github-actions)

A tiny, portable utility that measures sustainable memory bandwidth on Linux, macOS, and Windows.

📄 **Detailed benchmark specification:** [specs/benchmark-spec.md](./specs/benchmark-spec.md)

## Overview
The program implements the following benchmarks:

**Memory Bandwidth Benchmarks (STREAM-like):**
- **Copy**: `c[i] = a[i]` – measures raw read-write bandwidth
- **Triad**: `c[i] = a[i] + scalar * b[i]` – measures memory bandwidth with arithmetic
- **RandomRW**: Random access read/write – assesses random memory performance
- **ALU**: ALU-intensive operations – measures compute-bound performance

**SSD I/O Benchmarks:**
- **Sequential Read/Write**: Measures sustained sequential throughput
- **Random Read/Write**: Measures IOPS and latency for random access
- Configurable block sizes (1kB-4kB) for different workload simulations

All benchmarks are written in **C++17**, use only the C++ standard library, and are built with **CMake** for Linux, macOS, and Windows.

## Repository Layout
```
mem_band/
├─ CMakeLists.txt          # CMake build script
├─ README.md               # This file
├─ implementation-plan.md # High‑level implementation tasks
├─ specs/                  # Specification documents (markdown)
│   ├─ benchmark-spec.md   # Detailed benchmark description
│   └─ architecture-spec.md# Architecture considerations
├─ src/
│   ├─ main.cpp            # CLI, orchestration
│   ├─ benchmark.hpp       # Memory kernel implementations
│   ├─ ssd_benchmark.hpp   # SSD I/O benchmark implementations
│   └─ aligned_alloc.hpp   # Portable aligned allocation helpers
└─ tests/                  # Unit tests
    ├─ test_benchmark.cpp  # Basic kernel tests
    ├─ test_double.cpp     # Double precision tests
    ├─ test_alignment.cpp  # Alignment tests
    ├─ test_alu.cpp        # ALU kernel tests
    └─ test_ssd_benchmark.cpp  # SSD I/O benchmark tests
``` 
```
mem_band/
├─ CMakeLists.txt          # CMake build script
├─ README.md               # This file
├─ implementation-plan.md # High‑level implementation tasks
├─ specs/                  # Specification documents (markdown)
│   ├─ benchmark-spec.md   # Detailed benchmark description
│   └─ architecture-spec.md# Architecture considerations
├─ src/
│   ├─ main.cpp            # CLI, orchestration
│   ├─ benchmark.hpp       # Templated kernel implementations
│   └─ aligned_alloc.hpp   # Portable aligned allocation helpers
└─ scripts/                # Optional helper scripts (e.g., run_all.sh)
    └─ run_all.sh          # Example wrapper that runs several sizes
``` 

## Build Instructions
### Prerequisites
- **CMake** ≥ 3.15
- A C++ compiler supporting C++17 (gcc, clang, MSVC)

### Linux / macOS
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```
The resulting executable is `./mem_band`.

### Windows (PowerShell)
```powershell
mkdir build; cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```
The executable will be located at `Release\mem_band.exe`.

## Usage
```
./mem_band [options]

Options:
  -s, --size <MiB>       Size of each array (default: 256)
  -n, --iters <N>        Number of timed iterations per kernel (default: 20)
  -t, --type <float|double> Data type (default: float)
  -S, --simd             Enable SIMD (not implemented)
  -A, --alu              Run ALU-intensive kernel
  -I, --ssd              Run SSD I/O benchmark
  --ssd-path <path>      SSD benchmark directory (default: /tmp)
  --ssd-block <size>     SSD block size in bytes (default: 4096)
  --ssd-random           Random I/O vs sequential
  --ssd-read-only        Read-only SSD benchmark
  -R, --run-apu          Run APU system identifier collection
  -N, --run-npu          Run NPU benchmark
  --run-npu-suite        Run NPU benchmark suite (all precision/operation combinations)
  -M, --run-medium-test  Run only default test subset (excludes 1024 MiB stress test)
  -h, --help             Show this help message
```
### Memory Bandwidth Benchmark Example
```bash
./mem_band --size 1024 --iters 30 --type double
```
**Output:**
```
# Size: 1024 MiB, Type: double, Iterations: 30
Kernel   Bytes/Iter  Time(s)   Bandwidth(GB/s)
Copy     2.0e+09    0.62      3.23
Triad    3.0e+09    0.93      3.23
RandomRW 2.0e+09    0.34      0.41
```
### SSD I/O Benchmark Example
```bash
./mem_band --ssd --ssd-path /tmp --ssd-block 4096
```
**Output:**
```
# SSD I/O Benchmark
# Path: /tmp, Block Size: 4096 bytes, Random I/O: no

Benchmark     Bandwidth(MB/s)    IOPS      Latency(us)
SequentialWrite  4.02e+07   1.03e+07  0.097
```
### APU System Identifier Example
```bash
./mem_band --run-apu
```
**Output:**
```
# System Information
  CPU: AMD Ryzen AI 300 Series
  Platform: AMD Ryzen AI Strix Point
  Memory: 124546 MB
  OS: Ubuntu 24.04.4 LTS
```

### NPU Benchmark Example
```bash
./mem_band --run-npu
```
**Output:**
```
# NPU Benchmark

  Device: NPU 0
  Configuration: MatMul FP32 (size=1024)
  Metrics:
    Latency:      109.952 ms
    Throughput:   36954.5 OPS
                (3.86375e-08 TFLOPS)
    Power:        4.75633 W
```

### NPU Benchmark Suite Example
```bash
./mem_band --run-npu-suite
```
**Output:**
```
# Running NPU benchmark suite

  Device: NPU 0
  Configuration: MatMul FP32 (size=1024)
  Metrics:
    ...

  Device: NPU 0
  Configuration: MatMul FP16 (size=1024)
  Metrics:
    ...
```

## Interpreting Results

### Memory Bandwidth
- **Copy**: Measures raw read-write bandwidth (2× element size per element).
- **Triad**: Measures memory bandwidth with arithmetic operations.
- **RandomRW**: Measures random access performance (typically much lower than sequential).
- **ALU**: Measures compute-bound performance.
- As array size exceeds last-level cache, bandwidth typically plateaus. Use this plateau value as the system's sustainable memory bandwidth – a useful metric when evaluating hardware for AI workloads.

### SSD I/O
- **Sequential Read/Write**: Measures sustained throughput (typical for bulk data loading).
- **Random Read/Write**: Measures IOPS and latency (typical for database/cache workloads).
- Higher block sizes (4KB) better represent real-world file I/O.
- Lower latency values indicate faster response times.
- Compare results across storage types (HDD vs SSD vs NVMe) to understand storage bottlenecks.

## Extending the Benchmark
- Add multi‑threaded kernels (OpenMP / `std::thread`).
- Implement additional STREAM kernels (Scale, Add).
- Add non‑temporal (streaming) stores for systems that support them.
- Provide JSON or CSV output flags for automated data collection.

## Testing

### Memory Bandwidth Benchmark
```bash
./mem_band --size 64 --iters 2 --type float
```
Output should show Copy, Triad, RandomRW kernel results with bandwidth in GB/s.

### SSD I/O Benchmark
```bash
./build/test_ssd
```
Runs 9 unit tests for SSD benchmark functionality. Or test manually:
```bash
./mem_band --ssd --ssd-path /tmp --ssd-block 4096
```

### Full Test Suite
```bash
cd build && ctest --output-on-failure
```
All 10 tests should pass (including SSD tests).

### Quick Run (Medium Test Subset)
```bash
./mem_band --run-medium-test
```
Runs a default test subset excluding the 1024 MiB stress test.

## License
This project is released under the **MIT License** – see the `LICENSE` file for details.

## Legacy Platform Support

The benchmark supports legacy platforms including PowerPC32/64 and i386 Linux:

### Build on Legacy Platforms

**PowerPC32 (e.g., embedded PowerPC):**
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=toolchain-ppc32.cmake
```

**PowerPC64 (e.g., IBM Power systems):**
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=toolchain-ppc64.cmake
```

**i386 (32-bit x86):**
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=toolchain-i386.cmake
```

### Fallback Mechanisms

- **Aligned Allocation**: Uses `posix_memalign` as fallback on platforms without `_align_malloc` or `aligned_alloc`
- **SIMD Instructions**: Guarded by compiler-specific flags (`-maltivec` for PowerPC, `-msse2` for i386)
- **C++17 Compliance**: Falls back to standard C++ features when platform-specific optimizations are unavailable

### Platform Detection

The build system automatically detects the target platform and applies appropriate compiler flags. See the respective toolchain files for platform-specific configurations.
