# Implementation Plan

## Phase 1: Foundation
- [x] Create project directory structure
- [x] Initialize git repository
- [x] Create basic CMakeLists.txt
- [x] Set up src/ directory with placeholder files

## Phase 2: Memory Allocation
- [x] Implement aligned_alloc.hpp with platform-independent aligned allocation
- [x] Support for cache-line alignment (typically 64-byte)
- [x] Handle allocation failures gracefully
- Updated aligned_alloc to return nullptr on failure (no exceptions)
- [x] Provide aligned free function

## Phase 3: Benchmark Kernels
- [x] Implement templated Copy kernel in benchmark.hpp
- [x] Implement templated Scale kernel in benchmark.hpp
- [x] Implement templated Add kernel in benchmark.hpp
- [x] Implement templated Triad kernel in benchmark.hpp
- [x] Add optional SIMD vectorization support
- [x] Ensure numerical correctness of operations

## Phase 4: CLI & Orchestration
- [x] Implement command-line argument parsing in main.cpp
- [x] Support options: -s/--size, -n/--iters, -t/--type, -S/--simd, -R/--randomrw, -h/--help
- [x] Implement benchmark execution loop with timing
- [x] Calculate and format bandwidth results
- [x] Output results in CSV-friendly format

## Phase 5: Build System & CI/CD
- [x] Add global Makefile to simplify build/test commands
- [x] Configure CMakeLists.txt for C++17 and Release optimizations
- [x] Add GitHub Actions workflow (.github/workflows/ci.yml)
- [x] Install required dependencies in CI jobs (CMake, compilers)
- [x] Run unit tests (ctest) in CI and verify execution
- [x] Add status badge to README.md
- [x] Add CLI parameters for running test suites
  - `-R, --run-apu` - Run APU system identifier collection
  - `-N, --run-npu` - Run NPU benchmark
  - `--run-npu-suite` - Run NPU benchmark suite (all precision/operation combinations)
  - `-M, --run-medium-test` - Run only default test subset (excludes 1024 MiB stress test)

## Phase 6: Testing & Validation
- [x] Build and test on Linux/macOS/Windows
- [x] Validate output format matches specification
- [x] Add long-running CTest (1024 MiB) to ensure cache exhaustion

## Phase 7: Legacy Platform Support
- [x] Add CMake support for PowerPC32/64 and i386 Linux
- [x] Provide toolchain files (toolchain-ppc32.cmake, etc.)
- [x] Implement posox_memalign fallback in aligned_alloc.hpp
- [x] Guard SIMD flags (-maltivec for PPC, -msse2 for i386)
- [x] Update README.md with Legacy Support section

## Phase 8: Extended Benchmarks (GPU/SSD)
- [x] Add GPU memory bandwidth benchmark (CUDA/OpenCL)
- [x] Implement ALU intensive kernels (Integer/FP stress)
- [x] Implement SSD I/O tests (Sequential/Random, 1kB-4kB blocks)
- [x] Update main.cpp and documentation for GPU/ALU/SSD flags
- [x] Add APU (AMD Strix Point/Halo) support for memory bandwidth benchmarking
- [x] Add NPU (Neural Processing Unit) benchmark tests

## Phase 9: Documentation & Persistence
- [x] Add APU system identifier collection to implementation plan
- [x] Complete benchmark-spec.md and architecture-spec.md
- [x] Update README.md with clear usage and Legacy Support sections
- [x] Define system identifier collection (CPU model, memory size/type)
  - Created specs/system-identifier-spec.md with full specification
  - Implemented SystemInfo class in src/system_info.cpp
  - Platform: Linux/macOS/Windows detection
  - Compiler version detection
  - System ID hash generation
- [x] Add functionality to persist benchmark runs
  - Created CSV persistence mechanism in src/csv_output.hpp/cpp
  - Added CSVOutput class to append results to files
  - CSV format includes timestamp, system_id, kernel, size, type, bandwidth, latency
