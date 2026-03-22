# Memory Bandwidth Benchmark

A tiny, portable utility that measures sustainable memory bandwidth on Linux, macOS, and Windows.

## Overview
The program implements a subset of the well‑known **STREAM** benchmark kernels (Copy and Triad) and reports the achieved bandwidth in GB/s. It is written in **C++17**, uses only the C++ standard library, and is built with **CMake** to generate native build files for all three platforms.

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
  -S, --simd             Enable SIMD‑vectorised kernels (requires compiler support)
  -h, --help             Show this help message
```
Example:
```bash
./mem_band --size 1024 --iters 30 --type double
```
Output (tabular, CSV‑friendly):
```
# Size: 1024 MiB, Type: double, Iterations: 30
Kernel   Bytes/Iter  Time(s)   Bandwidth(GB/s)
Copy     2.0e+09    0.62      3.23
Triad    3.0e+09    0.93      3.23
```

## Interpreting Results
- **Copy** measures raw read‑write bandwidth (2 × element size per element).
- **Triad** adds a multiply‑add operation; bandwidth should be similar if the kernel is truly memory bound.
- As the array size exceeds the last‑level cache, the measured bandwidth typically plateaus. Use this plateau value as the system’s sustainable memory bandwidth – a useful metric when evaluating hardware for AI workloads.

## Extending the Benchmark
- Add multi‑threaded kernels (OpenMP / `std::thread`).
- Implement additional STREAM kernels (Scale, Add).
- Add non‑temporal (streaming) stores for systems that support them.
- Provide JSON or CSV output flags for automated data collection.

## License
This project is released under the **MIT License** – see the `LICENSE` file for details.
