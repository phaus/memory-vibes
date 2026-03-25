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
- [x] Provide aligned free function

## Phase 3: Benchmark Kernels
- [x] Implement templated Copy kernel in benchmark.hpp
- [x] Implement templated Triad kernel in benchmark.hpp
- [x] Implement templated Scale kernel in benchmark.hpp
- [x] Implement templated Add kernel in benchmark.hpp
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
- [x] Add GitHub Actions workflow (`.github/workflows/ci.yml`)
- [x] Install required dependencies in CI jobs (CMake, compilers)
- [x] Run unit tests (`ctest`) in CI and verify execution
- [x] Add status badge to README.md

## Phase 6: Testing & Validation
- [x] Build and test on Linux/macOS/Windows
- [x] Validate output format matches specification
- [ ] Add long-running CTest (1024 MiB) to ensure cache exhaustion

## Phase 7: Legacy Platform Support
- [ ] Add CMake support for PowerPC32/64 and i386 Linux
- [ ] Provide toolchain files (`toolchain-ppc32.cmake`, etc.)
- [ ] Implement `posix_memalign` fallback in `aligned_alloc.hpp`
- [ ] Guard SIMD flags (`-maltivec` for PPC, `-msse2` for i386)

## Phase 8: Extended Benchmarks (GPU/SSD)
- [ ] Add GPU memory bandwidth benchmark (CUDA/OpenCL)
- [ ] Implement ALU intensive kernels (Integer/FP stress)
- [ ] Implement SSD I/O tests (Sequential/Random, 1kB-4kB blocks)
- [ ] Update `main.cpp` and documentation for GPU/ALU/SSD flags

## Phase 9: Documentation
- [ ] Complete `benchmark-spec.md` and `architecture-spec.md`
- [ ] Update `README.md` with clear usage and Legacy Support sections

