# Implementation Plan

## High-Level Tasks

1. **Project Setup**
   - Initialize git repository
   - Create basic directory structure
   - Set up CMake configuration

2. **Core Implementation**
   - Implement portable aligned allocation helpers (`aligned_alloc.hpp`)
   - Implement templated benchmark kernels (`benchmark.hpp`)
   - Implement CLI and orchestration logic (`main.cpp`)

3. **Build System**
   - Configure CMakeLists.txt for cross-platform builds
   - Set up Release build configuration
   - Ensure proper compiler flags for C++17

4. **Testing & Validation**
   - Verify correct output format
   - Test on target platforms (Linux/macOS/Windows)
   - Validate benchmark results against expected behavior

5. **Documentation**
   - Complete README.md with usage instructions
   - Add specification documents (benchmark-spec.md, architecture-spec.md)
   - Ensure code comments and documentation are clear

## Detailed Task List

### Phase 1: Foundation
- [x] Create project directory structure
- [x] Initialize git repository
- [x] Create basic CMakeLists.txt
- [x] Set up src/ directory with placeholder files

### Phase 2: Memory Allocation
- [ ] Implement aligned_alloc.hpp with platform-independent aligned allocation
- [ ] Support for cache-line alignment (typically 64-byte)
- [ ] Handle allocation failures gracefully
- [ ] Provide aligned free function

### Phase 3: Benchmark Kernels
- [ ] Implement templated Copy kernel in benchmark.hpp
- [ ] Implement templated Triad kernel in benchmark.hpp
- [ ] Add optional SIMD vectorization support
- [ ] Ensure numerical correctness of operations

### Phase 4: CLI & Orchestration
- [ ] Implement command-line argument parsing in main.cpp
- [ ] Add support for all specified options (-s/--size, -n/--iters, -t/--type, -S/--simd, -h/--help)
- [ ] Implement benchmark execution loop with timing
- [ ] Calculate and format bandwidth results
- [ ] Output results in specified CSV-friendly format

### Phase 5: Build System
- [ ] Add global Makefile to simplify build and test commands
- [ ] Configure CMakeLists.txt for C++17 standard
- [ ] Set up proper include directories
- [ ] Configure Release build optimizations
- [ ] Ensure cross-platform compatibility

### Phase 6: Testing
- [ ] Build and test on Linux
- [ ] Build and test on macOS (if available)
- [ ] Build and test on Windows (if available)
- [ ] Validate output format matches specification
- [ ] Verify bandwidth calculations are correct
- [ ] Add longer‑running CTest (e.g., size 1024 MiB, many iterations) to ensure workload exceeds CPU cache
- Added basic CTest to run mem_band with minimal parameters and verify exit code

### Phase 7: Documentation
- [ ] Complete benchmark-spec.md with detailed specifications
- [ ] Complete architecture-spec.md with architectural decisions
- [ ] Update README.md with clear usage instructions
- [ ] Ensure all code is properly commented

## Dependencies
- CMake ≥ 3.15
- C++17 compatible compiler (GCC, Clang, or MSVC)

## Phase 9: CI/CD

- [ ] Add GitHub Actions workflow (`.github/workflows/ci.yml`) to build on Ubuntu, macOS, and Windows.
- [ ] Install required dependencies (CMake, compiler) in each job.
- [ ] Configure and build the project in Release mode.
- [ ] Run unit tests (`ctest`) and verify benchmark execution.
- [ ] Upload build artifacts or test results if needed.
- [ ] Add status badge to README.md.
## Phase 8: Extended Benchmarks
### GPU and ALU Tests
- [ ] Add GPU memory bandwidth benchmark (CUDA/OpenCL) for copy and compute kernels
- [ ] Implement ALU intensive kernels to stress integer/floating‑point units
- [ ] Provide CLI flags `--gpu` and `--alu` to enable these tests

### SSD I/O Benchmark
- [ ] Implement sequential read/write tests for block sizes 1 kB, 2 kB, 4 kB
- [ ] Implement random read/write tests for same block sizes
- [ ] Add CLI options `--ssd` with sub‑options `--seq`, `--rand`, and `--block-size <size>`
- [ ] Allow selection of target volumes via `--volumes <path1,path2,...>`
- [ ] Ensure proper flushing and cache eviction for accurate measurements

### Integration & Documentation
- [ ] Update argument parser in `main.cpp` to handle new options
- [ ] Extend README and usage docs with GPU/ALU/SSD sections
- [ ] Add validation checks for required libraries (CUDA, etc.)
- [ ] Provide example command lines for mixed workloads

- CMake ≥ 3.15
- C++17 compatible compiler (GCC, Clang, or MSVC)