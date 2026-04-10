# Memory Bandwidth Benchmark Specification

## Overview
This document specifies the behavior and implementation details of the memory bandwidth benchmark utility. The benchmark measures sustainable memory bandwidth on Linux, macOS, and Windows systems using a subset of the STREAM benchmark kernels.

## Supported Kernels
The benchmark implements the following kernels:

### Copy Kernel
- Operation: `c[i] = a[i]`
- Memory access pattern: Read from array `a`, write to array `c`
- Bytes per element: 2 × element size (one read, one write)

### Triad Kernel
- Operation: `c[i] = a[i] + scalar * b[i]`
- Memory access pattern: Read from arrays `a` and `b`, write to array `c`
- Bytes per element: 3 × element size (two reads, one write)

### RandomRW Kernel
- Operation: `c[idx[i]] = a[idx[i]]` where `idx` is a shuffled index vector
- Memory access pattern: Random reads from `a` and random writes to `c`
- Bytes per element: 2 × element size (one read, one write) per random access
- Used to assess random‑access memory performance

### ALU Kernel
- Operation: `temp = a[i] * b[i] + c[i]; a[i] = temp * (c[i] + 1);`
- Memory access pattern: Reads from arrays `a`, `b`, `c`; writes to array `a`
- Bytes per element: 4 × element size (three reads, one write) per element
- Used to assess ALU performance

### SSD I/O Benchmark
- Operation: Sequential and random read/write tests for storage devices
- Memory access pattern: File I/O with configurable block sizes (1kB-4kB)
- Metrics measured:
  - Bandwidth (MB/s)
  - IOPS (I/O Operations Per Second)
  - Latency (microseconds per I/O operation)
- Modes:
  - Sequential Write: Measures sequential write throughput
  - Sequential Read: Measures sequential read throughput
  - Random Write: Measures random write IOPS and throughput
  - Random Read: Measures random read IOPS and throughput
- Used to assess storage device performance for AI/ML workloads


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

### SSD I/O Benchmark (`-I`, `--ssd`)
- Enables SSD I/O benchmarking
- Additional options:
  - `--ssd-path <path>`: Directory for test files (default: /tmp)
  - `--ssd-block <size>`: Block size in bytes (default: 4096, range: 1024-4096)
  - `--ssd-random`: Enable random I/O instead of sequential
  - `--ssd-read-only`: Read-only test (requires pre-existing data)

### ALU Kernel (`-A`, `--alu`)
- Enables the ALU-intensive kernel alongside Copy, Triad, and RandomRW

### Accelerator Benchmarks
- `-R`, `--run-apu`: Collect APU system identifier info
- `-N`, `--run-npu`: Run NPU benchmark (mock implementation)
- `--run-npu-suite`: Run full NPU suite (all precisions and operation types)

### System Information
- `-P`, `--show-platform`: Show platform identification and exit
- `--show-features`: Show available runtime features and exit
- `-L`, `--system-layout`: Show system layout diagram and exit
- `--layout-format <fmt>`: Layout format: `text`, `mermaid`, `json` (default: text)

### Preset Modes
- `-M`, `--run-medium-test`: Medium test (256 MiB, standard iterations)
- `-Q`, `--quick-test`: Quick test (64 MiB, 5 iterations)

### Output Options
- `-o`, `--output-format <fmt>`: Output format: `text`, `csv`, `json` (default: text)
- `-f`, `--output-file <path>`: Write results to file (default: stdout)

## Output Format

### Memory Bandwidth Benchmark
The benchmark produces tabular, CSV-friendly output with the following format:

```
# Size: <size> MiB, Type: <type>, Iterations: <iterations>
Kernel   Bytes/Iter  Time(s)   Bandwidth(GB/s)
Copy     <bytes>     <time>    <bandwidth>
Triad    <bytes>     <time>    <bandwidth>
RandomRW <bytes>     <time>    <bandwidth>
ALU      <bytes>     <time>    <bandwidth>
```

### SSD I/O Benchmark
The SSD benchmark produces the following output:

```
# SSD I/O Benchmark
# Path: <path>, Block Size: <bytes> bytes, Random I/O: <yes|no>

Benchmark     Bandwidth(MB/s)    IOPS      Latency(us)
SequentialWrite <bw>    <iops>    <latency>
SequentialRead  <bw>    <iops>    <latency>
RandomRead      <bw>    <iops>    <latency>
RandomWrite     <bw>    <iops>    <latency>
```

Where:
- `<path>` is the benchmark directory
- `<bytes>` is the block size
- `<bw>` is the bandwidth in MB/s
- `<iops>` is the I/O operations per second
- `<latency>` is the average latency in microseconds

Where:
- `<size>` is the array size in MiB
- `<type>` is the data type (float or double)
- `<iterations>` is the number of timed iterations
- `<bytes>` is the number of bytes processed per iteration
- `<time>` is the average time per iteration in seconds
- `<bandwidth>` is the calculated bandwidth in GB/s

## Bandwidth Calculation

### Memory Bandwidth
Bandwidth is calculated using the formula:
```
bandwidth(GB/s) = (bytes_per_iteration) / (time_in_seconds) / 1e9
```

Where bytes_per_iteration depends on the kernel:
- Copy: 2 x array_size_in_bytes
- Triad: 3 x array_size_in_bytes
- RandomRW: 2 x array_size_in_bytes
- ALU: 4 x array_size_in_bytes

### SSD I/O Metrics
- **Bandwidth (MB/s)**: `(total_bytes / 1024²) / duration_seconds`
- **IOPS**: `num_blocks / duration_seconds`
- **Latency (µs)**: `(duration_seconds / num_blocks) × 10⁶`

Where:
- `total_bytes` = block_size × num_blocks
- `duration_seconds` is the measured time for all I/O operations
- `num_blocks` is the number of blocks processed

## Implementation Requirements
1. Must be written in C++17
2. Must use only the C++ standard library
3. Must be built with CMake to generate native build files
4. Must provide portable aligned allocation helpers
5. Must implement both Copy and Triad kernels
6. Must support configurable array size, iterations, and data type
7. Must optionally support SIMD vectorization
8. Must produce the specified output format
9. Must support SSD I/O benchmarking (sequential/random read/write)
10. Must support configurable block sizes (1kB-4kB)
11. Must report bandwidth, IOPS, and latency for SSD tests
12. Must be cross-platform compatible (Linux, macOS, Windows)