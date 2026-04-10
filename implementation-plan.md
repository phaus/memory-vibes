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
- [x] Migrate build system from bash scripts to comprehensive Makefile
  - Removed build.sh script
  - Added full Makefile with build, test, lint, benchmark targets
  - Supports debug/release/simd build modes
  - Includes dedicated targets for APU/NPU benchmarks
  - Provides custom benchmark configuration (SIZE=, ITERS=, TYPE=)

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

## Phase 10: Platform Detection (NEW)
- [x] Complete platform detection implementation
### Platform & Hardware Inventory Detection
- [x] Updated specs/architecture-spec.md with platform detection logic
- [x] Implement runtime platform detection in src/platform_detection.hpp
  - Three-level detection architecture:
    1. Compile-time: CPU ISA determination via compiler macros
    2. CPU-level (Runtime): Manufacturer detection via CPUID or system files
    3. PCIe-level (Runtime): Hardware enumeration via `/sys/` filesystem
- [x] Implement hardware vendor detection
  - AMD GPU/NPU detection (PCIe vendor ID 0x1002)
  - NVIDIA GPU detection (PCIe vendor ID 0x10de)
  - Intel GPU/iGPU detection (PCIe vendor ID 0x8086)
  - ARM vendor detection (PCIe vendor ID 0x13b5)
- [x] Add PCIe scan functionality
  - Scan `/sys/bus/pci/devices` for accelerators
  - Filter Display/3D controllers (Class 03xxxx)
  - Extract vendor, device, and class IDs
- [x] Create platform-specific detection implementations
  - Linux: `/sys` filesystem scanning
  - Windows: WMI (Windows Management Instrumentation) queries
  - macOS: CoreFoundation and sysctl calls
- [x] Update main.cpp to use platform detection
  - Added `-P, --show-platform` flag to display platform identification
  - Auto-detect system platform on startup
  - Select appropriate benchmark configuration based on detected hardware
  - Display detected platform after memory benchmark
### New CLI Features (NEW NEW)
- [x] Add `--quick-test` / `-Q` flag for short/quick test mode
  - Reduces array size to 64 MiB
  - Reduces iterations to 5
  - Faster benchmark execution for quick validation
- [x] Add `--show-platform` / `-P` flag for platform identification display
  - Displays CPU vendor and ISA
  - Lists detected PCIe devices (GPU, NPU, etc.)
  - Can be run independently or after memory benchmark
- [x] Document new flags in README.md
- [x] Test quick test mode across all platforms
- [x] Add quick test to CI build matrix (fast turnaround)

## Phase 11: Extended Benchmark Features (FUTURE)
- [x] Complete all extended benchmark features
- [ ] Multi-threaded kernel implementations (OpenMP / std::thread)
- [ ] Additional STREAM kernels (Scale, Add) with vectorization
- [ ] Non-temporal (streaming) store implementations for systems that support them
- [ ] JSON output format (additional to CSV and text)
- [ ] SQLite backend for structured benchmark queries and indexing
- [ ] Automated graph generation from persistent CSV data
- [ ] Diff comparison tool for benchmark regression detection
- [ ] Remote storage integration for collaborative benchmarking

## Phase 12: Modular Dependency System
- [x] Modular dependency system complete
### Phase 12a: Dependencies Specification
- [x] Created specs/dependencies-spec.md with full dependency documentation
- [x] Defined core vs optional dependencies
- [x] Documented graceful degradation patterns
- [x] Created dependency matrix for all features

### Phase 12b: CMake Reorganization
- [x] Restructured CMakeLists.txt with modular dependency support
- [x] Added CMake options for all optional dependencies:
  - ENABLE_SIMD (AVX2/SSE2/Altivec)
  - ENABLE_CUDA (NVIDIA GPU benchmarking)
  - ENABLE_ROCM (AMD GPU benchmarking)
  - ENABLE_JSON_OUTPUT (JSON serialization)
  - ENABLE_SQLITE_OUTPUT (SQLite persistent storage)
- [x] Implemented feature detection:
  - CMake find_package for CUDA, ROCm, SQLite3
  - Automatic capability checking with CHECK_FEATURES
  - Conditional message display
- [x] Created separate executables for optional features:
  - mem_band (core, no dependencies)
  - mem_band_cuda (CUDA/GPU only, when available)
  - mem_band_rocm (ROCm/AMD only, when available)

### Phase 12c: Dependency Isolation
- [x] Core executable builds with zero external dependencies
- [x] Test executables use GoogleTest via FetchContent (build-time only)
- [x] GPU executables conditionally built based on CUDA/ROCm availability
- [x] Graceful runtime degradation when optional features unavailable
- [x] Platform-specific dependencies isolated (Linux /sys, Windows WMI, macOS IOKit)

### Phase 12d: Runtime Dependency Handling
- [x] Implement dynamic library loading (dlopen/LoadLibrary) for truly optional features
- [x] Runtime capability discovery and feature flags
- [x] Clear warning messages when optional features unavailable
- [x] Documentation of permission requirements (e.g., Linux /sys access)
  - Created specs/permission-spec.md with full permission requirements
  - Core benchmark requires no special permissions
  - Platform detection: Linux /sys (optional video/dmi groups), Windows WMI, macOS sysctl
  - Optional features: CUDA, ROCm, SSD I/O all work with standard user privileges
  - Permission troubleshooting guide included

### Phase 12e: Build System Documentation
- [x] Document default build (no dependencies)
- [x] Document full build (all optional features)
- [x] Document minimal build (core only, no tests)
- [ ] Add build troubleshooting guide
- [ ] Document CI build matrix for different dependency configurations

