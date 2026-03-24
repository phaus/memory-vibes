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

### Phase 7: Documentation
- [ ] Complete benchmark-spec.md with detailed specifications
- [ ] Complete architecture-spec.md with architectural decisions
- [ ] Update README.md with clear usage instructions
- [ ] Ensure all code is properly commented

## Dependencies
- CMake ≥ 3.15
- C++17 compatible compiler (GCC, Clang, or MSVC)