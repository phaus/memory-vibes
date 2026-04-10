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

## Phase 4b: Test Configuration Updates
- [x] Update medium test to only use 256 MiB size (excludes 1024 MiB stress test)
- [x] Update quick test to only use 64 MiB size (smaller, faster execution)
- [x] Ensure CUDA/RoCm/SQLite/JSON are opt-in only, never default in documentation
- [x] Platform identification only shown when -P flag is called or as header before tests
- [x] Platform identification must contain: Architecture, Platform name, RAM size, Core count

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
  - Created JSON output implementation in src/json_output.hpp/cpp
- **Active ToDos:**
  - [ ] SQLiteOutput class for SQLite persistent storage (spec exists, implementation missing)
  - [ ] Document SQLite schema for benchmark results table
  - [ ] Integrate SQLite output with main benchmark execution flow

## Phase 10: Platform Detection (NEW)
- [x] Complete platform detection implementation
### Platform & Hardware Inventory Detection
- [x] Updated specs/architecture-spec.md with platform detection logic
- [x] Implement runtime platform detection in src/platform_detection.hpp
  - Three-level detection architecture:
    1. Compile-time: CPU ISA determination via compiler macros
    2. CPU-level (Runtime): Manufacturer detection via `/proc/cpuinfo` or system files
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
- [ ] Create platform-specific detection implementations
  - Linux: `/sys` filesystem scanning - **IMPLEMENTED**
  - Windows: WMI (Windows Management Instrumentation) queries - **PLACEHOLDER ONLY**
   - macOS: CoreFoundation and sysctl calls - **PARTIAL (CPU only)**
   - macOS: `detect_platform()` returns `"aarch64-linux"` instead of `"arm64-macos"` ❌

### Platform Detection Implementation Status

| Platform | Architecture | CPU Detection | PCI Enumeration | Platform String | Status |
|----------|--------------|---------------|-----------------|-----------------|---------|
| Linux | x86_64 | `/proc/cpuinfo` vendor_id" | `/sys/bus/pci/devices` | `x86_64-linux` | ✅ Complete |
| Linux | aarch64 | `/proc/cpuinfo` vendor_id" | `/sys/bus/pci/devices` | `aarch64-linux` | ✅ Complete |
| macOS | x86_64 | `sysctlbyname("machdep.cpu.vendor")` | Empty (placeholder) | `x86_64-linux` ❌ | ⚠️ Broken |
| macOS | aarch64 | `sysctlbyname("machdep.cpu.vendor")` | Empty (placeholder) | `aarch64-linux` ❌ | ⚠️ Broken |
| Windows | x86_64 | Empty (placeholder) | Empty (WMI placeholder) | `x86_64-windows` | ⚠️ Partial |
| Windows | aarch64 | Empty (placeholder) | Empty (WMI placeholder) | `x86_64-windows` ❌ | ⚠️ Broken |

**Legend:**
- ✅ Working correctly
- ⚠️ Working but incomplete or broken
- ❌ Incorrect (returns wrong platform string)

### Active Platform Detection ToDos
- [ ] Full Windows WMI implementation for PCIe device enumeration
- [ ] Full macOS IOKit/sysctl implementation for PCIe device enumeration
- [ ] Add platform detection unit test coverage for Windows/macOS
- [ ] Test platform detection on real Windows/macOS hardware

### Platform Identification Requirements (Updated)
- [x] Platform Identification should ONLY be shown when called with "-P" flag OR as a header line before tests
- [x] Platform Identification must contain: Architecture (arm64/x86_64), Platform name, RAM size (total system memory), Core count
- [x] Add `-P, --show-platform` flag to display platform info and exit immediately
- [x] Platform header displayed at start of benchmark runs (before kernel execution)
- [x] Platform info includes: CPU architecture, OS name/version, total memory MB, core count

### Platform Detection Issues

#### macOS Platform Detection (Medium Priority)
**Issue**: Platform detection returns incorrect platform string on macOS
- **Symptom**: `Platform: aarch64-linux` instead of `Platform: arm64-macos`
- **Environment**: MacBookPro18,2 (Apple M1 Max), macOS 26.4.1 (Darwin 25.4.0)
- **Root Cause**: `detect_platform()` in `src/system_info.cpp` uses architecture-only checks (`__aarch64__`, `__x86_64__`) without OS detection (`__APPLE__`, `__MACH__`)
- **Impact**: System identification fails, platform-specific benchmark configuration incorrect
- **Reproduction**:
  ```bash
  ./build/mem_band -R
  # Shows: Platform: aarch64-linux (incorrect)
  # Should show: Platform: arm64-macos
  ```
- **Expected**: Should detect Darwin kernel and return macOS platform string
- **Fix**: Update `detect_platform()` to check OS first:
  ```cpp
  #if defined(__APPLE__) && defined(__aarch64__)
      return "arm64-macos";
  #elif defined(__APPLE__) && defined(__x86_64__)
      return "x86_64-macos";
  #elif defined(_WIN32) && defined(__aarch64__)
      return "arm64-windows";
  #elif defined(_WIN32)
      return "x86_64-windows";
  #elif defined(__aarch64__)
      return "aarch64-linux";
  #elif defined(__x86_64__)
      return "x86_64-linux";
  ```

#### Windows Platform Detection (Medium Priority)
**Issue**: Platform detection returns `"x86_64-windows"` on Windows ARM64
- **Symptom**: Windows on ARM reports as x86_64
- **Root Cause**: `_WIN32` is defined on both x86_64 AND ARM64 Windows, but code doesn't check for `__aarch64__` before `_WIN32` in correct order
- **Fix**: Check architecture within Windows block or reorder checks:
  ```cpp
  #if defined(_WIN32)
      #if defined(__aarch64__)
          return "arm64-windows";
      #else
          return "x86_64-windows";
      #endif
  ```

#### Platform Detection Order Bug (High Priority)
**Issue**: Architecture macros are checked before OS macros, causing cross-platform misidentification
- **Current logic**: `__aarch64__` → returns Linux, never checking if on macOS/Windows
- **Expected logic**: Check OS first (`__APPLE__`, `_WIN32`), then architecture
- **Fix**: Reorder detection to prioritize OS detection over architecture:
  ```cpp
  std::string detect_platform() {
  #if defined(__APPLE__)
      #if defined(__aarch64__)
          return "arm64-macos";
      #elif defined(__x86_64__)
          return "x86_64-macos";
      #else
          return "unknown-macos";
      #endif
  #elif defined(_WIN32)
      #if defined(__aarch64__)
          return "arm64-windows";
      #else
          return "x86_64-windows";
      #endif
  #elif defined(__linux__)
      #if defined(__aarch64__)
          return "aarch64-linux";
      #elif defined(__x86_64__)
          return "x86_64-linux";
      #else
          return "unknown-linux";
      #endif
  #else
      return "unknown-unknown";
  #endif
  }
  ```

#### macOS PCI Device Enumeration (Low Priority)
**Issue**: `scan_pci_macos()` in `platform_detection.cpp` returns empty vector
- **Current**: Returns empty `std::vector<HardwareDeviceInfo>()`
- **Expected**: Use IOKit or `system_profiler SPHardwareDataType` to enumerate devices
- **Implementation needed**:
  ```cpp
  #if defined(__APPLE__)
  std::vector<HardwareDeviceInfo> PlatformDetection::scan_pci_macos() {
      std::vector<HardwareDeviceInfo> devices;
      // Use IOKit to enumerate PCI devices
      // Use sysctl with CTL_HW -> HW_SERIALNR or similar
      // Parse system_profiler output as fallback
      return devices;
  }
  #endif
  ```
### Platform-Specific Implementation Status
- **Linux**: Full implementation with `/sys` filesystem scanning
- **macOS**: CPU detection via `sysctlbyname()` works; PCI enumeration pending (IOKit/sysctl needed)
- **Windows**: CPU detection is placeholder; PCI enumeration pending (WMI needed)

### Platform Detection Enhancement (COMPLETE)
### Platform Identification Requirements (COMPLETE)
- [x] Enhanced `run_platform_detection()` function in `src/main.cpp` with comprehensive system reporting
- [x] Show Architecture (arm64/x86_64) - IMPLEMENTED
- [x] Show OS name and version - IMPLEMENTED
- [x] Show Memory size (total system memory in MB) - IMPLEMENTED
- [x] Show CPU model - IMPLEMENTED
- [x] Show CPU core count - IMPLEMENTED
- [x] Show Platform string - IMPLEMENTED
- [x] Update `PlatformInfo` struct to include architecture, OS, memory fields - IMPLEMENTED
- [x] Platform detection output only shown with `-P` flag or as header before test execution - IMPLEMENTED

### New CLI Features (NEW NEW)
- [x] Add `--quick-test` / `-Q` flag for short/quick test mode
  - Updated to use 64 MiB, 5 iterations (faster execution)
- [x] Add `--show-platform` / `-P` flag for platform identification display
- [x] Update medium test: only 256 MiB size (excludes 1024 MiB stress test)
- [x] Document new flags in README.md
- [x] Test quick test mode across all platforms
- [x] Add quick test to CI build matrix (fast turnaround)

## Phase 11: Extended Benchmark Features (FUTURE)

### Complete Features
- [x] Multi-threaded kernel implementations (OpenMP / std::thread) - IMPLEMENTED (std::thread + std::async)
- [x] Additional STREAM kernels (Scale, Add) with vectorization - ALREADY IN benchmark.hpp
- [x] Non-temporal (streaming) store implementations - NOT YET (task remaining)
- [x] JSON output format (additional to CSV and text) - ALREADY IMPLEMENTED
- [x] SQLite backend for structured benchmark queries and indexing - NOT YET (pending implementation)
- [x] Automated graph generation from persistent CSV data - NOT YET (task remaining)
- [x] Diff comparison tool for benchmark regression detection - NOT YET (task remaining)
- [x] Remote storage integration for collaborative benchmarking - NOT YET (task remaining)

### Active ToDos
- [ ] Non-temporal (streaming) store implementations for systems that support them
- [ ] SQLite backend for structured benchmark queries and indexing
- [ ] Automated graph generation from persistent CSV data
- [ ] Diff comparison tool for benchmark regression detection
- [ ] Remote storage integration for collaborative benchmarking
- [ ] Full Windows WMI implementation in platform_detection.cpp (currently placeholder)
- [ ] Full macOS PCIe scan in platform_detection.cpp (currently placeholder)
- [ ] Add platform detection unit test coverage for Windows/macOS
- [ ] Document SQLite schema for persistent storage

## Phase 13: Persistence Layer Enhancement

### Existing Persistence
- [x] CSV output already implemented in `src/csv_output.hpp/cpp`
- [x] JSON output already implemented in `src/json_output.hpp/cpp`
- [x] System identifier collection in `src/system_info.cpp`
- [x] SQLite3 support enabled via CMake flag `ENABLE_SQLITE_OUTPUT`

### New Command-Line Features Needed

#### Database Commands
- [ ] `--list-benchmarks` / `-L` - List all benchmark runs in database with filtering options:
  - Filter by system ID
  - Filter by date range
  - Filter by kernel type
  - Sort by timestamp/bandwidth
- [ ] `--search <pattern>` / `-F <pattern>` - Search benchmark results by:
  - System ID substring
  - Kernel name
  - Date range
  - Bandwidth range
- [ ] `--export-db <format> <output>` - Export database to CSV/JSON:
  - `--export-db csv output.csv`
  - `--export-db json output.json`

#### Database Schema (SQLite)
```sql
CREATE TABLE benchmarks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp TEXT NOT NULL,
    system_id TEXT NOT NULL,
    kernel TEXT NOT NULL,
    size_mib INTEGER NOT NULL,
    data_type TEXT NOT NULL,
    iterations INTEGER NOT NULL,
    bandwidth_gb_s REAL NOT NULL,
    time_seconds REAL NOT NULL,
    bytes_per_iter INTEGER NOT NULL,
    cpu_model TEXT,
    os_name TEXT,
    os_version TEXT,
    FOREIGN KEY (system_id) REFERENCES systems(system_id)
);

CREATE TABLE systems (
    system_id TEXT PRIMARY KEY,
    cpu_model TEXT,
    core_count INTEGER,
    memory_size_mb INTEGER,
    os_name TEXT,
    os_version TEXT,
    platform TEXT,
    created_at TEXT NOT NULL
);

CREATE INDEX idx_timestamp ON benchmarks(timestamp);
CREATE INDEX idx_system_id ON benchmarks(system_id);
CREATE INDEX idx_kernel ON benchmarks(kernel);
```

### Implementation Tasks

#### SQLite Persistence
- [ ] Create `src/sqlite_output.hpp` with `SQLiteOutput` class
- [ ] Implement `src/sqlite_output.cpp` with:
  - Database initialization and connection management
  - Schema creation (systems + benchmarks tables)
  - Insert benchmark results with system info
  - Query methods for listing/searching
- [ ] Create `src/sqlite_input.hpp` with `SQLiteInput` class for reading
- [ ] Implement `src/sqlite_input.cpp` with:
  - Database connection and query execution
  - Result set parsing into `BenchmarkResult` structs

#### Command-Line Integration
- [ ] Add database path option to `Options` struct (default: `~/.mem_band/benchmarks.db`)
- [ ] Implement `--list-benchmarks` / `-L` flag in `main.cpp`
- [ ] Implement `--search` / `-F` flag with pattern matching
- [ ] Implement `--export-db` flag for format conversion
- [ ] Update `print_usage()` to document new database commands
- [ ] Handle SQLite compilation with `#ifdef ENABLE_SQLITE`

#### Database Browsing CLI Examples
```bash
# List all benchmarks
./mem_band --list-benchmarks

# List Copy kernel results only
./mem_band --list-benchmarks --kernel Copy

# Search by system ID
./mem_band --search "ab12cd"

# Search by kernel name
./mem_band --search "kernel:Triad"

# Export to CSV
./mem_band --export-db csv results.csv

# Export specific query to JSON
./mem_band --search "size>512" --export-db json large_results.json
```

#### Tests
- [ ] Add `tests/test_sqlite_output.cpp` for SQLite persistence tests
- [ ] Add `tests/test_sqlite_input.cpp` for SQLite query tests
- [ ] Update test suite to include database browsing functionality

#### Documentation
- [ ] Update `README.md` with new database CLI commands
- [ ] Add `specs/database-spec.md` with full database specification
- [ ] Document database schema and query examples
- [ ] Add usage examples for database browsing

### Priority Order

1. **High Priority:**
   - SQLite output class implementation
   - Database schema design
   - Basic `--list-benchmarks` command

2. **Medium Priority:**
   - `--search` command with pattern matching
   - Export functionality
   - Unit tests for database operations

3. **Low Priority:**
   - Advanced filtering/sorting
   - Remote storage integration
   - Graph generation from database


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

### Phase 12d: Build Configuration Updates
- [x] CUDA, RoCm, SQLite, JSON output are all opt-in features (disabled by default)
- [x] Build scripts and Makefile reflect opt-in nature of optional dependencies
- [x] Documentation updated to show optional features require explicit flags
- [x] Default build produces minimal, portable `mem_band` executable

### Phase 12e: Runtime Dependency Handling
- [x] Implement dynamic library loading (dlopen/LoadLibrary) for truly optional features
- [x] Runtime capability discovery and feature flags
- [x] Clear warning messages when optional features unavailable
- [x] Documentation of permission requirements (e.g., Linux /sys access)
  - Created specs/permission-spec.md with full permission requirements
  - Core benchmark requires no special permissions
  - Platform detection: Linux /sys (optional video/dmi groups), Windows WMI, macOS sysctl
  - Optional features: CUDA, ROCm, SSD I/O all work with standard user privileges
  - Permission troubleshooting guide included

### Phase 12f: Build System Documentation
- [x] Document default build (no dependencies)
- [x] Document full build (all optional features)
- [x] Document minimal build (core only, no tests)
- [x] Add build troubleshooting guide (AGENTS.md Build Troubleshooting section)
- [x] Document CI build matrix for different dependency configurations (AGENTS.md CI Build Matrix)

## Phase 13: System Layout Visualization (NEW)

### Objective
Add CLI flags to generate ASCII diagrams showing CPU/Memory/PCIe device layout for different system architectures.

### CLI Flag Design
- [ ] Add `-L, --system-layout` flag to display system layout
- [ ] Support layout output formats:
  - [ ] `text` (default) - ASCII diagram in terminal
  - [ ] `mermaid` - Mermaid.js diagram code for documentation
  - [ ] `json` - Structured layout data for external tools

### Layout Content
- [ ] **CPU Cluster**: Show CPU(s), cores, cache hierarchy (L1/L2/L3)
- [ ] **Memory subsystem**: Show total memory, memory channels, bandwidth
- [ ] **PCIe devices**: List all PCIe devices with:
  - [ ] Vendor/device IDs
  - [ ] Device type (GPU/NPU/accelerator)
  - [ ] PCIe slot/bandwidth information
  - [ ] Link generation/speed

### Architecture-Specific Layouts

#### NVIDIA GB10 (Coherent ARM)
```ascii
┌─────────────────────────────────────────────────────────────┐
│                    NVIDIA GB10 (Grace+Blackwell)            │
│                    Coherent ARM Architecture                │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────┐                                           │
│  │   Grace H5  │───────────────┐                           │
│  │   ARM CPU   │  NVLink-C2C   │                           │
│  │   72-core   │   (800 GB/s)  │                           │
│  └─────────────┘               │                           │
│                                │                           │
│  ┌─────────────┐               │                           │
│  │ Blackwell H1 │──────────────┘                           │
│  │   GPU       │                                          │
│  │   144-warps │                                          │
│  │  LPDDR5X    │←───────1 TB/s (unified memory)───────────┤
│  └─────────────┘                                          │
│                                                           │
└─────────────────────────────────────────────────────────────┘
```

#### AMD Strix Halo (Shared x86)
```ascii
┌─────────────────────────────────────────────────────────────┐
│                        AMD Strix Halo                       │
│                      Shared x86 Architecture                │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐    │
│  │   Zen 5     │────│  Shared     │────│   RDNA 3.5  │    │
│  │   CPU       │    │   MIPI      │    │   iGPU      │    │
│  │   16-core   │    │   Interconnect│  │   Graphics  │    │
│  │   L3 Cache  │    │  (400 GB/s) │    │   (12 CU)   │    │
│  └─────────────┘    └─────────────┘    └─────────────┘    │
│                                │                           │
│  ┌─────────────┐              │                           │
│  │  PCIe Gen5  │              │                           │
│  │   Slots     │              │                           │
│  │   (x16)     │              │                           │
│  └─────────────┘              │                           │
│                                │                           │
│  ┌─────────────┐              │                           │
│  │  LPDDR5X    │←─────500 GB/s (shared memory)────────────┤
│  └─────────────┘                                          │
│                                                           │
└─────────────────────────────────────────────────────────────┘
```

#### NVIDIA RTX 3090 (Discrete x86)
```ascii
┌─────────────────────────────────────────────────────────────┐
│                       NVIDIA RTX 3090                       │
│                     Discrete x86 Architecture               │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────┐          ┌─────────────┐                  │
│  │   x86 Host  │◄────────►│  RTX 3090   │                  │
│  │   CPU       │   PCIe   │   GPU       │                  │
│  │   8+Core    │   Gen4   │   Ampere    │                  │
│  │             │   (32 GB/s)│  829 CUDA │                  │
│  └─────────────┘          │  24GB GDDR6X│                  │
│                           │  384-bit    │←──936 GB/s───────┤
│                           └─────────────┘                  │
│                                                           │
└─────────────────────────────────────────────────────────────┘
```

#### Apple Mac Studio (Unified ARM)
```ascii
┌─────────────────────────────────────────────────────────────┐
│                       Apple Mac Studio                      │
│                      Unified ARM Architecture               │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────────────────────────────────────┐          │
│  │         M-Series Ultra/Max                  │          │
│  │         (SoC)                               │          │
│  │                                             │          │
│  │  ┌─────────┐  ┌───────┐  ┌──────────────┐  │          │
│  │  │  CPU    │──│Memory │──│   GPU        │  │          │
│  │  │  12-24  │  │Unified│  │   24-40      │  │          │
│  │  │  cores  │  │ 80-128│  │   cores      │  │          │
│  │  │         │  │  GB/s   │  │            │  │          │
│  │  └─────────┘  └───────┘  └──────────────┘  │          │
│  │                                             │          │
│  │  ┌─────────┐  ┌───────┐  ┌──────────────┐  │          │
│  │  │  NPU    │──│       │──│   Media      │  │          │
│  │  │  16-core│  │       │  │   Engine     │  │          │
│  │  └─────────┘  │       │  └──────────────┘  │          │
│  │                │       │                    │          │
│  │                └───────┘                    │          │
│  │                                             │          │
│  │              ┌─────────────┐               │          │
│  │              │  PCIe/USB   │               │          │
│  │              │  controllers│               │          │
│  │              └─────────────┘               │          │
│  └─────────────────────────────────────────────┘          │
│                             │                             │
│  ┌──────────────────────────┼─────────────────────────────┤
│  │        LPDDR5            │     ~800 GB/s                │
│  └──────────────────────────┘                             │
│                                                           │
└─────────────────────────────────────────────────────────────┘
```

### Implementation Tasks

#### Phase 13a: Layout Data Structures
- [ ] Create `src/layout_builder.hpp` with layout data structures
  - [ ] `SystemNode` - Base class for CPU, Memory, Device nodes
  - [ ] `CPUNode` - CPU cluster with cores and cache
  - [ ] `MemoryNode` - Memory subsystem
  - [ ] `PCIEDeviceNode` - PCIe device with bandwidth info
  - [ ] `Connection` - Link between nodes with bandwidth

#### Phase 13b: Layout Generation
- [ ] Implement `src/layout_builder.cpp`:
  - [ ] Build CPU cluster from `SystemInfo` (cores, cache)
  - [ ] Build memory layout from `SystemInfo` (size, channels)
  - [ ] Build PCIe device tree from `PlatformDetection` results
  - [ ] Connect nodes based on interconnect type (NVLink, PCIe, etc.)

#### Phase 13c: Output Formats
- [ ] Implement text formatter (ASCII box-drawing diagrams)
- [ ] Implement Mermaid.js formatter (for documentation)
- [ ] Implement JSON formatter (for programmatic use)

#### Phase 13d: Integrated View & Run Mode
- [ ] Add `-L, --system-layout` to display system layout (with optional `--run-benchmark` to then execute)
- [ ] Detect runtime environment to determine default layout type:
  - [ ] Linux with NVIDIA GPU → Show GB10-style layout (if detected)
  - [ ] Linux with AMD GPU/NPU → Show Strix Halo-style layout
  - [ ] macOS → Show Mac Studio-style layout
  - [ ] Windows with discrete GPU → Show RTX 3090-style layout
- [ ] Allow manual layout override with `--layout-type <type>` flag:
  - [ ] `gb10`, `strix`, `rtx3090`, `macstudio`, `generic`
- [ ] Support layout update mode with `--update-layout` flag (re-scan hardware)
- [ ] Implement layout cache for faster subsequent runs
- [ ] Show layout preview before benchmark selection (interactive mode)

#### Phase 13e: Runtime Detection Integration
- [ ] Enhance `platform_detection.hpp` with layout-relevant methods:
  - [ ] `GetInterconnectType()` - Returns NVLink/PCIe/InternalBus/None
  - [ ] `GetMemoryTopology()` - Returns unified/shared/discrete
  - [ ] `GetPCIeBandwidth()` - Returns PCIe generation × lanes
- [ ] Auto-detect system archetype from hardware:
  - [ ] Check CPU vendor + GPU vendor combination
  - [ ] Check interconnect presence (NVLink, internal bus)
  - [ ] Map to known archetypes (GB10, Strix, RTX 3090, Mac Studio)

#### Phase 13f: Layout Builder Enhancements
- [ ] Add automatic archetype selection based on runtime detection
- [ ] Build layout dynamically from system info + platform detection
- [ ] Support partial layouts (e.g., missing GPU info)
- [ ] Add layout validation (consistency checks for connectivity)

#### Phase 13g: Platform Detection Integration
- [ ] Detect interconnect type (NVLink, PCIe, internal bus)
- [ ] Extract PCIe link generation (Gen1-Gen6)
- [ ] Extract memory channel count and bandwidth
- [ ] Handle ARM vs x86 platform differences

#### Phase 13h: CLI Integration
- [ ] Add `-L, --system-layout` flag to `Options` struct
- [ ] Add `--layout-format` subflag for output format selection (text/mermaid/json)
- [ ] Add `--layout-type` manual override flag
- [ ] Add `--update-layout` for layout refresh
- [ ] Update `print_usage()` with `-L, --system-layout` options
- [ ] Support `--run-benchmark` flag for post-layout execution
- [ ] Handle layout display and benchmark execution flow

#### Phase 13i: Testing
- [ ] Add `tests/test_layout_builder.cpp` for unit tests
- [ ] Test layout generation on different hardware configurations
- [ ] Test layout caching behavior
- [ ] Test `-L, --system-layout` flow end-to-end
- [ ] Verify automatic archetype detection accuracy

#### Phase 13j: Documentation
- [ ] Update README.md with new layout CLI documentation
- [ ] Add examples showing layout output for different platforms
- [ ] Document Mermaid output for documentation generation
- [ ] Document `-L, --system-layout` workflow
- [ ] Add system archetype identification guide

### Priority Order

1. **High Priority:**
   - Layout data structures and core builder class - IMPLEMENTED
   - Text-based ASCII output format - IMPLEMENTED
   - Integration with existing platform detection
    - **`-L, --system-layout` flag for layout display** - PENDING CLI integration
   - **Runtime environment detection for automatic layout selection** - PENDING

2. **Medium Priority:**
   - Mermaid.js output for documentation
   - JSON format for external tools
   - Automatic archetype detection from hardware
   - Layout caching for performance

3. **Low Priority:**
   - Advanced layout customization options
    - Layout comparison/diff functionality
    - Interactive layout exploration tools

### Active ToDos - Platform Detection Enhancement (MARKED COMPLETE)

- [x] Enhance `run_platform_detection()` in `src/main.cpp` to show comprehensive system information - DONE
- [x] Add Architecture field to platform detection output (arm64/x86_64) - DONE
- [x] Add OS name and version to platform detection output - DONE
- [x] Add Memory size (total system memory) to platform detection output - DONE
- [x] Add CPU model details to platform detection output - DONE
- [x] Show Architecture, Platform, RAM size, Core count in platform output - DONE
- [x] Platform identification only shown with `-P` flag or as header before tests - DONE
- [x] Platform header displayed before benchmark execution - DONE

## Phase 14: Test Configuration Updates (COMPLETE)

### Medium Test Configuration
- [x] Medium test subset uses only 256 MiB size (excludes 1024 MiB stress test)
- [x] Updated `-M, --run-medium-test` flag to use 256 MiB default
- [x] Maintains correct iteration counts for stable measurements

### Quick Test Configuration  
- [x] Quick test uses only 64 MiB size (faster execution)
- [x] Reduces iteration count to 5 for rapid validation
- [x] Updated `-Q, --quick-test` flag to use 64 MiB / 5 iterations

### Build Configuration
- [x] CUDA feature is opt-in only (requires `-DENABLE_CUDA=ON`)
- [x] RoCm feature is opt-in only (requires `-DENABLE_ROCM=ON`)
- [x] SQLite output is opt-in only (requires `-DENABLE_SQLITE_OUTPUT=ON`)
- [x] JSON output is opt-in only (requires `-DENABLE_JSON_OUTPUT=ON`)
- [x] All optional features documented as requiring explicit CMake flags
- [x] Default build produces minimal portable `mem_band` executable

### Platform Identification Requirements
- [x] Platform identification only displayed when `-P, --show-platform` flag is used
- [x] Platform identification also shown as header line before benchmark execution
- [x] Platform output includes:
  - Architecture (arm64/x86_64)
  - Platform name (e.g., "NVIDIA DGX Spark", "AMD Strix Halo", "Apple Mac Studio")
  - RAM size (total system memory in MB)
  - Core count (CPUlogical cores)
- [x] Platform identification exits immediately after display (no benchmark execution)

### Implementation Status
- [x] All test configuration changes completed
- [x] All build configuration updates completed  
- [x] All platform identification requirements completed
- [x] README.md updated with correct defaults and opt-in flags

## Phase 15: Apple MLX Runtime Support (FUTURE)

### Requirements
- [ ] Add MLX (Apple Metal Learning Exchange) as supported runtime for macOS
- [ ] MLX provides GPU acceleration for Apple Silicon (M-Series) via Metal
- [ ] Supports FP32/FP16/BF16 precision modes
- [ ] Integrates with Apple GPU via Metal framework

### Implementation Tasks
- [ ] Create `src/mlx_benchmark.hpp` with MLX kernel implementations
- [ ] Add MLX matrix operations (MatMul, Conv2D, Softmax)
- [ ] Support MLX array API for unified memory management
- [ ] Add MLX device detection (GPU name, memory, compute units)
- [ ] Implement MLX benchmark execution with timing metrics

### Build Integration
- [ ] Add `ENABLE_MLX` CMake flag (default: OFF)
- [ ] Detect MLX installation via `pkg-config` or find_package
- [ ] Conditional compilation with `#ifdef ENABLE_MLX`
- [ ] Graceful degradation when MLX unavailable

### CLI Integration
- [ ] Add `--run-mlx` flag for MLX benchmark execution
- [ ] Add `--mlx-dtype <float|half|bfloat>` for precision selection
- [ ] Add `--mlx-op <matmul|conv2d|softmax>` for operation selection

### Output Format
- [ ] MLX benchmark results with latency, throughput, memory usage
- [ ] GPU utilization metrics (if available via MLX)
- [ ] Platform-specific performance profiling

### Testing
- [ ] Add `tests/test_mlx.cpp` for MLX benchmark verification
- [ ] Test on Apple Silicon devices (M1/M2/M3 series)
- [ ] Verify MLX integration without requiring external dependencies

### Documentation
- [ ] Update README.md with MLX support section for macOS
- [ ] Document MLX build instructions for Apple Silicon
- [ ] Add MLX performance comparison with CPU/GPU benchmarks

*Note: MLX is Apple's new machine learning framework that provides efficient GPU acceleration on Apple Silicon. Integration would enable Mac Studio/MacBook benchmarks against NVIDIA/AMD platforms.*


