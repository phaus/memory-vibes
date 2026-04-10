# Memory Bandwidth Benchmark

![Generated with local AI](https://img.shields.io/badge/Generated%20with%20local%20AI-blue) ![Hardware](https://img.shields.io/badge/Hardware-NVIDIA%20DGX%20Spark%20GB10-blue) ![Model](https://img.shields.io/badge/Model-Qwen--Qwen3--5--35B--FP8-blue) ![CI Build Status](https://img.shields.io/github/actions/workflow/status/phaus/memory-vibes/ci.yml?branch=main&logo=github-actions)

A tiny, portable utility that measures sustainable memory bandwidth on Linux, macOS, and Windows.

📄 **Detailed benchmark specification:** [specs/benchmark-spec.md](./specs/benchmark-spec.md)
📄 **System identifier & persistence spec:** [specs/system-identifier-spec.md](./specs/system-identifier-spec.md)
📄 **Hardware test lab spec:** [specs/hardware-test-lab-spec.md](./specs/hardware-test-lab-spec.md)
📄 **Permission requirements spec:** [specs/permission-spec.md](./specs/permission-spec.md)

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
- **Configurable** block sizes (1kB-4kB) for different workload simulations

**APU/NPU Benchmarking:**
- **APU System Identifier**: Collects hardware/system information for benchmark tracking
- **NPU Benchmark**: Neural Processing Unit performance testing (FP32/FP16, MatMul/Conv2D)
- **NPU Suite**: Full benchmark suite across all precision/operation combinations

All benchmarks are written in **C++17**, use only the C++ standard library, and are built with **CMake** for Linux, macOS, and Windows.

## Repository Layout
```
mem_band/
├─ CMakeLists.txt          # CMake build script
├─ README.md               # This file
├─ implementation-plan.md  # High-level implementation tasks
├─ specs/                  # Specification documents (markdown)
│   ├─ benchmark-spec.md      # Detailed benchmark description
│   ├─ architecture-spec.md   # Architecture considerations
│   ├─ dependencies-spec.md   # Project dependencies
│   ├─ hardware-test-lab-spec.md # Hardware test lab recommendations
│   ├─ permission-spec.md     # Permission requirements
│   ├─ stream-spec.md         # STREAM benchmark reference
│   └─ system-identifier-spec.md # System ID & persistence spec
├─ src/
│   ├─ main.cpp              # CLI, orchestration, benchmark dispatch
│   ├─ benchmark.hpp         # Templated kernel implementations
│   ├─ aligned_alloc.hpp     # Portable aligned allocation helpers
│   ├─ ssd_benchmark.hpp     # SSD I/O benchmark implementations
│   ├─ apu_identifier.hpp    # APU system identifier collection
│   ├─ npu_benchmark.hpp     # NPU benchmark implementations
│   ├─ gpu_benchmark.hpp     # GPU benchmark (optional, CUDA)
│   ├─ system_info.hpp/.cpp  # System information (mem_band namespace)
│   ├─ platform_detection.hpp/.cpp  # Platform detection
│   ├─ runtime_detection.hpp/.cpp   # Runtime feature detection
│   ├─ layout_builder.hpp/.cpp      # System layout diagram builder
│   ├─ json_output.hpp/.cpp  # JSON output formatting
│   ├─ csv_output.hpp/.cpp   # CSV output formatting
│   └─ benchmark_result.hpp  # Shared benchmark result struct
└─ tests/                  # Unit tests (GoogleTest)
    ├─ test_benchmark.cpp  # Basic kernel tests
    ├─ test_double.cpp     # Double precision tests
    ├─ test_alignment.cpp  # Alignment tests
    ├─ test_alu.cpp        # ALU kernel tests
    ├─ test_ssd_benchmark.cpp  # SSD I/O benchmark tests
    ├─ test_apu_identifier.cpp # APU system identifier tests
    ├─ test_npu.cpp        # NPU benchmark unit tests
    ├─ test_system_info.cpp    # System info tests
    ├─ test_layout_builder.cpp # Layout builder tests
    └─ test_layout_cli.cpp     # Layout CLI tests
```

## Build Instructions
### Prerequisites
- **CMake** ≥ 3.15
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

Memory benchmark options:
  -s, --size <MiB>           Array size in MiB (default: 256)
  -n, --iters <N>            Iterations per kernel (default: 20)
  -t, --type <float|double>  Data type (default: float)
  -S, --simd                 Enable SIMD kernels (requires build flag)
  -A, --alu                  Include ALU-intensive kernel

SSD benchmark options:
  -I, --ssd                  Run SSD I/O benchmark
  --ssd-path <path>          Benchmark directory (default: /tmp)
  --ssd-block <bytes>        Block size (default: 4096)
  --ssd-random               Use random I/O (default: sequential)
  --ssd-read-only            Read-only benchmark

Accelerator benchmarks:
  -R, --run-apu              Collect APU system identifier info
  -N, --run-npu              Run NPU benchmark
  --run-npu-suite            Run full NPU suite (all precisions/ops)

System information:
  -P, --show-platform        Show platform identification and exit
  --show-features            Show available runtime features and exit
  -L, --system-layout        Show system layout diagram and exit
  --layout-format <fmt>      Layout format: text, mermaid, json (default: text)

Preset modes:
  -M, --run-medium-test      Medium test (256 MiB, standard iterations)
  -Q, --quick-test           Quick test (64 MiB, 5 iterations)

Output options:
  -o, --output-format <fmt>  Output format: text, csv, json (default: text)
  -f, --output-file <path>   Write results to file (default: stdout)

  -h, --help                 Show this help message
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

### ALU-Intensive Kernel Example
```bash
./mem_band --size 1024 --iters 30 --alu
```
**Output:**
```
# Size: 1024 MiB, Type: float, Iterations: 30
Kernel   Bytes/Iter  Time(s)   Bandwidth(GB/s)
Copy     2097152000    0.62      3.23
Triad    3145728000    0.93      3.23
RandomRW 2097152000    0.34      0.41
ALU      4194304000    0.15      2.80
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

### Random SSD I/O Example
```bash
./mem_band --ssd --ssd-path /tmp --ssd-block 4096 --ssd-random
```
**Output:**
```
# SSD I/O Benchmark
# Path: /tmp, Block Size: 4096 bytes, Random I/O: yes

Benchmark     Bandwidth(MB/s)    IOPS      Latency(us)
RandomRead      2.15e+05   5.56e+04  17.85
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

### NPU Performance
- **Latency**: Lower is better for inference tasks
- **Throughput**: Higher OPS (operations per second) indicates better performance
- **Power**: Lower power consumption for same throughput indicates efficiency

## Extending the Benchmark
Already implemented extensions:
- Additional STREAM kernels (Scale, Add, ALU) in `benchmark.hpp`
- JSON and CSV output via `--output-format` flag
- System layout diagrams via `-L/--system-layout`
- Runtime feature detection via `--show-features`

Planned future extensions:
- Multi-threaded kernels (OpenMP / `std::thread`)
- Non-temporal (streaming) stores for systems that support them
- SQLite persistent storage backend

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

### APU System Identifier Tests
```bash
./build/test_apu_identifier
```
Runs APU system identifier tests across platforms.

### NPU Benchmark Tests
```bash
./build/test_npu
```
Runs NPU benchmark unit tests.

### Full Test Suite
```bash
cd build && ctest --output-on-failure
```
All tests should pass (including SSD, APU, and NPU tests).

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

## Hardware Test Lab

The benchmark is designed to run across four fundamental architectural archetypes to provide comprehensive memory performance insights:

### Supported Architectures

| Platform | Arch Type | Interconnect | Memory Type | Target Bandwidth |
| :--- | :--- | :--- | :--- | :--- |
| **NVIDIA GB10** | Coherent ARM | NVLink-C2C | LPDDR5X (Unified) | > 1 TB/s |
| **AMD Strix Halo** | Shared x86 | Internal Bus | LPDDR5X (Shared) | ~500 GB/s |
| **NVIDIA RTX 3090** | Discrete x86 | PCIe Gen4 | GDDR6X (Local) | ~936 GB/s |
| **Apple Mac Studio** | Unified ARM | UltraFusion | LPDDR5 (Unified) | ~800 GB/s |

### Why These Architectures?

**NVIDIA GB10 (DGX Spark)** - The Coherent Datacenter Giant
- ARM (Grace) + NVIDIA (Blackwell) via NVLink-C2C
- Tests hardware-level coherency and ultra-fast interconnects
- Methodology: NVLink / coherent memory access

**AMD Strix Halo (Ryzen AI Max)** - The Performance APU
- x86 (Zen 5) + RDNA 3.5 Integrated Graphics
- Stresses shared memory contention between CPU and iGPU
- Methodology: Unified Memory with CPU-GPU contention analysis

**NVIDIA RTX 3090 (24GB)** - The Enthusiast Evergreen
- Discrete Ampere GPU via PCIe Gen4
- 24GB VRAM with 384-bit GDDR6X bus (~936 GB/s)
- Methodology: Explicit PCIe data transfers

**Apple Mac Studio** - The Unified Workstation
- ARM (M-Series Ultra/Max) with up to 800 GB/s unified memory
- Enables large-scale LLM inference beyond traditional VRAM limits
- Methodology: Zero-copy unified memory access

### Data Movement Methodologies

| Architecture | Methodology | Key Focus |
|--------------|-------------|-----------|
| RTX 3090 | `cudaMemcpy` | PCIe transfer overhead |
| Strix Halo | ROCm Unified Memory | DRAM contention |
| Mac Studio | Metal Storage Buffer | Zero-copy efficiency |
| GB10 | NVLink / BusGrind | Hardware coherency |

### Implementation Roadmap

1. **Develop on RTX 3090**: Establish the "Discrete Baseline"
2. **Verify on Strix Halo**: Adjust for "Shared Memory" methodology
3. **Optimize for Mac Studio/GB10**: Refine for ARM unified/coherent logic
4. **Final Stress Test**: Simultaneous benchmarks on Strix Halo to measure CPU-GPU contention

### Lab Value Coverage

| Capability | Provided By |
| :--- | :--- |
| x86 Ecosystem | Strix Halo, RTX 3090 (Host) |
| ARM Ecosystem | GB10, Mac Studio |
| AI Community | RTX 3090, Mac Studio |
| Enterprise Future | GB10 |
| High-End Desktop | Mac Studio, Strix Halo |

---

📄 **Full hardware test lab specification:** [specs/hardware-test-lab-spec.md](./specs/hardware-test-lab-spec.md)
