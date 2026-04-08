# Memory Bandwidth Benchmark Architecture

## High-Level Architecture
The memory bandwidth benchmark follows a modular design separating concerns into distinct components:

### Components
1. **Main Orchestration** (`src/main.cpp`) – Handles command‑line argument parsing, benchmark execution coordination (including the RandomRW kernel), and output formatting
2. **Benchmark Kernels** (`src/benchmark.hpp`) – Templated implementations of the Copy, Triad, and RandomRW kernels
3. **Memory Allocation** (`src/aligned_alloc.hpp`) – Portable aligned memory allocation helpers for proper cache‑line alignment

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

## Platform Detection

This document describes the runtime approach to detect the underlying hardware platform without external dependencies (like `libpci`).

### Architecture
Detection occurs across three levels:
1. **Compile-Time**: Determine CPU_instruction set architecture (ISA) via compiler macros
2. **CPU-Level (Runtime)**: Query manufacturer (Intel, AMD, ARM Implementer) via CPUID or system files
3. **PCIe-Level (Runtime)**: Scan `/sys/` filesystem on Linux to identify discrete accelerators (GPUs, NPUs) via vendor IDs

### C Reference Implementation

The following implementation requires only a standard compiler (GCC/Clang) and a Linux system.

```c
// inventory.h
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
    #include <cpuid.h>
#endif

typedef enum {
    ARCH_X86_64,
    ARCH_ARM64,
    ARCH_UNKNOWN
} cpu_arch_t;

typedef enum {
    VENDOR_AMD    = 0x1002,
    VENDOR_NVIDIA = 0x10de,
    VENDOR_INTEL  = 0x8086,
    VENDOR_ARM    = 0x13b5
} pci_vendor_t;

// Helper function to read hex values from /sys
unsigned int read_hex(const char* path) {
    unsigned int val = 0;
    FILE* f = fopen(path, "r");
    if (f) {
        fscanf(f, "%x", &val);
        fclose(f);
    }
    return val;
}

// 1. CPU Architecture via macros
cpu_arch_t get_arch() {
#if defined(__x86_64__) || defined(_M_X64)
    return ARCH_X86_64;
#elif defined(__aarch64__) || defined(_M_ARM64)
    return ARCH_ARM64;
#else
    return ARCH_UNKNOWN;
#endif
}

// 2. PCIe Scan via /sys/bus/pci/devices
void scan_pci_inventory() {
    const char* pci_path = "/sys/bus/pci/devices";
    DIR* dir = opendir(pci_path);
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char path[512];
        snprintf(path, sizeof(path), "%s/%s/vendor", pci_path, entry->d_name);
        unsigned int vendor = read_hex(path);
        
        snprintf(path, sizeof(path), "%s/%s/device", pci_path, entry->d_name);
        unsigned int device = read_hex(path);

        snprintf(path, sizeof(path), "%s/%s/class", pci_path, entry->d_name);
        unsigned int class_id = read_hex(path);

        // Filter: Only Display/3D Controller (Class 03xxxx)
        if ((class_id >> 16) == 0x03) {
            printf("Found Accelerator: [%04x:%04x] at %s (Class %06x)\n", 
                    vendor, device, entry->d_name, class_id);
            
            switch(vendor) {
                case VENDOR_NVIDIA: printf(" -> Target: NVIDIA\n"); break;
                case VENDOR_AMD:    printf(" -> Target: AMD\n"); break;
                case VENDOR_INTEL:  printf(" -> Target: Intel\n"); break;
            }
        }
    }
    closedir(dir);
}
```

### Linux
- Uses standard POSIX APIs where applicable
- Relies on C++17 standard library features
- Builds with GCC or Clang compilers
- `/sys` filesystem for hardware enumeration

### Windows
- Compatible with MSVC compiler
- Uses CMake generator for Visual Studio
- Handles differences in memory allocation APIs
- WMI (Windows Management Instrumentation) for hardware detection

### macOS
- Uses BSD APIs and sysctl for system information
- Built with Clang compiler
- CoreFoundation for hardware enumeration

### AMD Platform Support

#### AMD GPUs
- **Tool Integration**: Use `rocm_bandwidth_test` for host–GPU bandwidth measurement
  - Supports Host → Device, Device → Host, and Device → Device transfers
  - Implements copy- and kernel-based test methodologies
- **Device Memory Bandwidth**: Profile with `rocprof-compute` for roofline analysis and memory throughput
- **Profiling**: Utilize `rocprofv3` for tracing and counters, `rocprof-sys` for system-level analysis

#### AMD APUs
- ROCm available:
  - Use `rocm_bandwidth_test` for transfer bandwidth
- Fallback mechanisms:
  - Custom streaming kernel for DRAM bandwidth measurement
  - ROCm profiling tools (`rocprof-compute`, `rocprof-sys`) for system analysis
- **Architectural Considerations**:
  - APUs use shared memory architecture (no separate VRAM)
  - NUMA topology and memory controller placement significantly impact bandwidth results
  - Results vary based on memory channel configuration and NUMA node affinity

---

#### NVIDIA GPUs
- **Tool Integration**: Use CUDA Demo Suite tools:
  - `bandwidthTest`: Host ↔ Device and Device ↔ Device transfers
    - Tests both page-able and pinned (page-locked) memory
  - `busGrind`: Multi-GPU bandwidth analysis
    - PCIe and NVLink interconnect evaluation
- **Device Memory Bandwidth**: Profile with Nsight Compute
  - Kernel-level memory workload analysis
  - Bottleneck identification for memory-bound kernels
- **CUDA Best Practices**:
  - Leverage pinned memory for realistic PCIe bandwidth measurements
  - Distinguish between copy-bandwidth (transfer) and kernel-bandwidth (compute)
  - Account for NVLink vs PCIe differences in multi-GPU setups

#### NVIDIA Grace / GH200 (GPU-Hybrid APUs)
- **Measurement Approach**:
  - Focus on kernel-based DRAM bandwidth measurement
  - Use Nsight Compute for unified memory profiling
  - PCIe tests often not representative for unified memory architectures
- **Architectural Considerations**:
  - Unified memory architecture requires different measurement methodology
  - GPU and CPU share physical memory pools
  - Memory controller and interconnect topology critical for performance

---

#### Intel Discrete GPUs
- **Tool Integration**:
  - **Intel VTune Profiler**: GPU analysis and memory usage view
    - Identify GPU hotspots
    - Analyze memory hierarchy and bandwidth limitations
  - **Intel Advisor**: GPU Roofline analysis
    - Memory-bound workload visualization
    - Memory hierarchy analysis (DRAM, L3 Cache, Shared Local Memory)
    - Bottleneck identification
- **Transfer Bandwidth**: Custom SYCL/Level Zero benchmark implementations
  - No direct equivalent to CUDA bandwidthTest
  - Implementation requires Level One API integration

#### Intel APUs / iGPUs
- **Primary Tools**:
  - Intel Advisor Roofline analysis
  - VTune GPU Analysis
- **Measurement Focus**:
  - Memory hierarchy analysis rather than raw GB/s metrics
  - Evaluate DRAM, L3 cache, and cache-line interactions
  - Shared memory architecture between CPU and GPU cores
- **Architectural Considerations**:
  - Shared DRAM between CPU and GPU cores
  - Cache hierarchy performance critical for overall bandwidth
  - Simple memcpy tests insufficient—kernel-based measurements required
  - Integrated graphics uses system memory with limited bandwidth isolation

---

#### ARM CPUs (Host DRAM)
- **Primary Benchmark**: STREAM Benchmark (de-facto standard for CPU-to-DRAM bandwidth)
  - Measures Copy, Scale, Add, and Triad kernels
  - Relevant for ARM Neoverse (server) and Cortex (client) CPUs
- **Verification Tools**:
  - `lscpu`: Verify memory hierarchy and topology
  - **PMUs (Performance Monitoring Units)**: ARM Developer documentation for memory event measurement
    - Low-cycle-accurate counters for memory bandwidth tracking
    - Hardware performance counters for precise measurements

#### ARM SoCs with Integrated GPUs (Mali, Adreno, Immortalis)
- **Profiling Tools**:
  - **ARM Mobile Studio - Streamline Performance Analyzer**: Visualizes CPU/GPU counters and actual memory traffic on system interconnect
  - **Graphics Analyzer**: Analyzes API calls and memory bottlenecks
- **Measurement Approach**:
  - Compute microbenchmarks via OpenCL or Vulkan Compute shaders
  - Buffer copy and Triad kernels since no dedicated VRAM exists
  - Direct access to shared memory through compute kernels

#### ARM APU/SoC Architectural Considerations
- **Unified Memory Architecture**:
  - CPU and GPU share physical DRAM
  - "Transfers" are typically ownership changes or cache flushes, not PCIe-based copying
  - Memory bandwidth measured through compute kernels, not host-device transfers
- **Cache Coherency**:
  - Measurements heavily influenced by I/O coherence support
  - I/O coherent GPUs: Transparent coherency, kernel-based measurement straightforward
  - Non-coherent GPUs: Manual cache maintenance required, adds complexity to benchmark
- **Thermal Considerations**:
  - ARM systems often throttle faster under sustained memory load compared to desktop systems
  - Burst measurements differ significantly from sustained bandwidth values
  - Multi-stage measurements recommended to capture thermal throttling effects
  - Extended benchmark runs may show declining bandwidth as system thermals limit performance

## Extensibility Points
The architecture is designed to allow for future extensions:
1. Additional STREAM kernels (Scale, Add) can be added to `benchmark.hpp`
2. Multi-threading support can be added via OpenMP or std::thread
3. Non-temporal store implementations can be added as specialized kernel variants
4. Alternative output formats (JSON, CSV) can be added to the output formatting in main.cpp