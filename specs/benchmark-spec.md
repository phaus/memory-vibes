# Memory Bandwidth Benchmark Specification

## Overview
This document specifies the behavior and implementation details of the memory bandwidth benchmark utility. The benchmark measures sustainable memory bandwidth on Linux, macOS, and Windows systems using a subset of the STREAM benchmark kernels.

## Supported Kernels
The benchmark implements two kernels from the STREAM benchmark suite:

### Copy Kernel
- Operation: `c[i] = a[i]`
- Memory access pattern: Read from array `a`, write to array `c`
- Bytes per element: 2 × element size (one read, one write)

### Triad Kernel
- Operation: `c[i] = a[i] + scalar * b[i]`
- Memory access pattern: Read from arrays `a` and `b`, write to array `c`
- Bytes per element: 3 × element size (two reads, one write)

## Configuration Options
The benchmark accepts the following command-line options:

### Size (`-s`, `--size`)
- Specifies the size of each array in MiB
- Default value: 256 MiB
- Must be a positive integer

### Iterations (`-n`, `--iters`)
- Specifies the number of timed iterations per kernel
- Default value: 20
- Must be a positive integer

### Data Type (`-t`, `--type`)
- Specifies the data type for array elements
- Supported values: `float` (default), `double`
- Determines element size: 4 bytes for float, 8 bytes for double

### SIMD Vectorization (`-S`, `--simd`)
- Enables SIMD-vectorised kernels when supported by the compiler
- Requires compiler support for SIMD instructions
- When disabled, uses scalar implementations

## Output Format
The benchmark produces tabular, CSV-friendly output with the following format:

```
# Size: <size> MiB, Type: <type>, Iterations: <iterations>
Kernel   Bytes/Iter  Time(s)   Bandwidth(GB/s)
Copy     <bytes>     <time>    <bandwidth>
Triad    <bytes>     <time>    <bandwidth>
```

Where:
- `<size>` is the array size in MiB
- `<type>` is the data type (float or double)
- `<iterations>` is the number of timed iterations
- `<bytes>` is the number of bytes processed per iteration
- `<time>` is the average time per iteration in seconds
- `<bandwidth>` is the calculated bandwidth in GB/s

## Bandwidth Calculation
Bandwidth is calculated using the formula:
```
bandwidth(GB/s) = (bytes_per_iteration) / (time_in_seconds) / 1e9
```

Where bytes_per_iteration depends on the kernel:
- Copy: 2 × array_size_in_bytes
- Triad: 3 × array_size_in_bytes

## Implementation Requirements
1. Must be written in C++17
2. Must use only the C++ standard library
3. Must be built with CMake to generate native build files
4. Must provide portable aligned allocation helpers
5. Must implement both Copy and Triad kernels
6. Must support configurable array size, iterations, and data type
7. Must optionally support SIMD vectorization
8. Must produce the specified output format