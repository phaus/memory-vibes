# Memory Bandwidth Benchmark Architecture

## High-Level Architecture
The memory bandwidth benchmark follows a modular design separating concerns into distinct components:

### Components
1. **Main Orchestration** (`src/main.cpp`) - Handles command-line argument parsing, benchmark execution coordination, and output formatting
2. **Benchmark Kernels** (`src/benchmark.hpp`) - Templated implementations of the Copy and Triad STREAM kernels
3. **Memory Allocation** (`src/aligned_alloc.hpp`) - Portable aligned memory allocation helpers for proper cache-line alignment

### Data Flow
1. Command-line arguments are parsed in `main.cpp`
2. Arrays are allocated using aligned allocation helpers
3. Benchmark kernels are executed for the specified number of iterations
4. Timing measurements are collected for each kernel
5. Bandwidth calculations are performed and results are formatted for output

## Key Design Decisions

### Templated Kernels
The benchmark kernels are implemented as templates to support different data types (float, double) without code duplication. This allows the same algorithm to work with different element sizes while maintaining type safety.

### Portable Aligned Allocation
Memory alignment is critical for performance, especially when using SIMD instructions. The aligned allocation helper ensures proper cache-line alignment regardless of platform, using platform-specific APIs when available and falling back to standard allocation with manual alignment when necessary.

### Separation of Concerns
By separating the CLI handling, kernel implementations, and memory allocation, the codebase remains maintainable and each component can be tested or modified independently.

### Build System
CMake is used as the build system to provide consistent builds across Linux, macOS, and Windows platforms while allowing platform-specific optimizations when beneficial.

## Platform Considerations

### Linux/macOS
- Uses standard POSIX APIs where applicable
- Relies on C++17 standard library features
- Builds with GCC or Clang compilers

### Windows
- Compatible with MSVC compiler
- Uses CMake generator for Visual Studio
- Handles differences in memory allocation APIs

## Extensibility Points
The architecture is designed to allow for future extensions:
1. Additional STREAM kernels (Scale, Add) can be added to `benchmark.hpp`
2. Multi-threading support can be added via OpenMP or std::thread
3. Non-temporal store implementations can be added as specialized kernel variants
4. Alternative output formats (JSON, CSV) can be added to the output formatting in main.cpp