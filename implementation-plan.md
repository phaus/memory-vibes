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
  - [x] SQLiteOutput class implemented in src/sqlite_output.hpp/cpp
  - [x] SQLite schema documented (systems + benchmarks tables with indexes)
  - [x] SQLite output integrated into main benchmark execution flow (run_sqlite_output called after memory benchmarks)
  - [x] Verified timestamp roundtrip: sqlite_output writes `"%Y-%m-%d %H:%M:%S"`, sqlite_input reads it correctly
  - [x] Removed dead code: main.cpp:492-493 `br.timestamp` assignment (sqlite_output ignores it, uses own `now()`)
  - [x] Removed redundant `#define ENABLE_SQLITE` from top of sqlite_output.cpp and sqlite_input.cpp
  - [x] Fixed CMake bug: test_sqlite_output and test_sqlite_input were missing `ENABLE_SQLITE` compile definition — tests compiled but GoogleTest never ran due to `#ifdef ENABLE_SQLITE` guards
  - [ ] Populate systems table with real values (core_count, memory_size_mb, platform currently hardcoded to 0/"") — **pass through `SystemInfo::collect()` via `run_sqlite_output()`**
  - [ ] Add transaction wrapping to SQLiteOutput::append() for bulk writes
  - [ ] Add thread-safety (mutex) to SQLite I/O for concurrent benchmark runs
  - [ ] Proper CSV quoting in csv_output and SQLiteInput::export_csv()
  - [ ] Proper JSON escaping in SQLiteInput::export_json() (currently raw string concatenation)

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
- [x] Create platform-specific detection implementations
   - Linux: `/sys` filesystem scanning - **IMPLEMENTED**
   - macOS: CoreFoundation/IOKit sysctl calls - **IMPLEMENTED**
   - macOS: PCI enumeration via IOKit device tree - **IMPLEMENTED**
   - Windows: CPU vendor detection via `__cpuid` intrinsic - **IMPLEMENTED**
   - Windows: PCI scanning - returns empty (future: WMI/SetupAPI)

### Platform Detection Implementation Status

| Platform | Architecture | CPU Detection | PCI Enumeration | Platform String | Status |
|----------|--------------|---------------|-----------------|-----------------|---------|
| Linux | x86_64 | `/proc/cpuinfo` vendor_id" | `/sys/bus/pci/devices` | `x86_64-linux` | Complete |
| Linux | aarch64 | `/proc/cpuinfo` vendor_id" | `/sys/bus/pci/devices` | `aarch64-linux` | Complete |
| macOS | x86_64 | `sysctlbyname("machdep.cpu.vendor")` | IOKit device tree | `x86_64-macos` | Complete |
| macOS | aarch64 | `sysctlbyname("machdep.cpu.vendor")` | IOKit device tree | `arm64-macos` | Complete |
| Windows | x86_64 | `__cpuid` intrinsic | WMI + SetupAPI fallback | `x86_64-windows` | Complete |
| Windows | aarch64 | `__cpuid` intrinsic | WMI + SetupAPI fallback | `arm64-windows` | Complete |

**Note**: Platform detection order bug (architecture checked before OS) has been **FIXED** in both `system_info.cpp` and `apu_identifier.hpp`. OS is now checked first, then architecture.

### Active Platform Detection ToDos
- [ ] Test platform detection on real Windows/macOS hardware
- [ ] Cross-platform CI testing for Windows/macOS detection paths (platform-independent test coverage)

### Platform Identification Requirements (Updated)
- [x] Platform Identification should ONLY be shown when called with "-P" flag OR as a header line before tests
- [x] Platform Identification must contain: Architecture (arm64/x86_64), Platform name, RAM size (total system memory), Core count
- [x] Add `-P, --show-platform` flag to display platform info and exit immediately
- [x] Platform header displayed at start of benchmark runs (before kernel execution)
- [x] Platform info includes: CPU architecture, OS name/version, total memory MB, core count

### Platform Detection Issues (Resolved)

#### macOS Platform Detection - FIXED
- `detect_platform()` in `system_info.cpp` and `detect_apu_platform()` in `apu_identifier.hpp` now use OS-first ordering
- macOS correctly returns `arm64-macos` or `x86_64-macos`

#### Windows Platform Detection - FIXED
- `detect_platform()` now checks `_WIN32` before architecture, with nested `__aarch64__`/`_M_ARM64` check
- Windows ARM64 correctly returns `arm64-windows`

#### Platform Detection Order Bug - FIXED
- Detection now prioritizes OS detection (`__APPLE__`, `_WIN32`, `__linux__`) before architecture

#### macOS PCI Device Enumeration (Low Priority)
**Issue**: `scan_pci_macos()` in `platform_detection.cpp` returns empty vector
- **Current**: Returns empty `std::vector<HardwareDeviceInfo>()`
- **Expected**: Use IOKit or `system_profiler SPHardwareDataType` to enumerate devices
### Platform-Specific Implementation Status

| Platform | CPU Detection | PCI Enumeration | Platform String | Notes |
|----------|--------------|-----------------|-----------------|-------|
| Linux | `/proc/cpuinfo` vendor_id | `/sys/bus/pci/devices` full scan | `x86_64-linux`, `aarch64-linux` | Complete, functional |
| macOS | `sysctlbyname("machdep.cpu.vendor")` | IOKit `IOPCIDevice` service tree | `x86_64-macos`, `arm64-macos` | CPU detection works; PCI populates `vendor_id` from device tree |
| Windows | `__cpuid` intrinsic | Empty stub (returns `[]{}`) | `x86_64-windows`, `arm64-windows` | CPU detection functional; PCI needs WMI/SetupAPI (low priority) |
| Cross-platform | — | — | Fallback: `unknown-unknown` | ARM returns `"ARM 64-bit CPU"` with no actual model |

### Known Platform Detection Issues
- **macOS PCI scan**: Uses IOKit but only populates `vendor_id` from `CFData` bytes; `vendor` string may be empty
- **Windows PCI scan**: Returns empty vector — needs WMI/SetupAPI for real enumeration (low priority)
- **Windows system_info**: `detect_cpu_model()`, `detect_memory_size()`, `detect_os()` only have `__linux__`/`__APPLE__` guards — no Windows implementation
- **ARM CPU model**: `detect_cpu_model()` returns literal `"ARM 64-bit CPU"` with no actual model identification
- **simd_enabled**: Hardcoded to `true` in `SystemIdentifier` regardless of compile-time or runtime availability

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
- [x] Non-temporal (streaming) store implementations - IMPLEMENTED in benchmark.hpp
- [x] JSON output format (additional to CSV and text) - IMPLEMENTED in json_output.hpp/cpp
- [x] SQLite backend for structured benchmark queries and indexing - IMPLEMENTED (sqlite_output + sqlite_input)
- [x] --list-benchmarks CLI flag - IMPLEMENTED (gated behind ENABLE_SQLITE)
- [x] --search <pattern> CLI flag - IMPLEMENTED (gated behind ENABLE_SQLITE)
- [x] --export-db <format> <file> CLI flag - IMPLEMENTED (gated behind ENABLE_SQLITE)
- [ ] Automated graph generation from persistent CSV data - NOT YET IMPLEMENTED
- [ ] Diff comparison tool for benchmark regression detection - NOT YET IMPLEMENTED
- [ ] Remote storage integration for collaborative benchmarking - NOT YET IMPLEMENTED

  ### Active ToDos
  - [x] Non-temporal (streaming) store implementations for systems that support them
  - [x] SQLite backend fully implemented (read + write + CLI integration)
  - [x] Removed dead br.timestamp code in main.cpp
  - [x] Fixed CMake ENABLE_SQLITE definition for test targets (tests were silently not running)
  - [x] Verified timestamp roundtrip works correctly
  - [ ] Automated graph generation from persistent CSV data
  - [ ] Diff comparison tool for benchmark regression detection
  - [ ] Remote storage integration for collaborative benchmarking
  - [ ] Fix timestamp format mismatch: sqlite_output writes `"%Y-%m-%d %H:%M:%S"` but sqlite_input reads as milliseconds epoch
  - [ ] Populate systems table with real values (core_count, memory_size_mb, platform are hardcoded to 0/"")
  - [ ] Add thread-safety (mutex) to SQLite I/O for concurrent benchmark runs
  - [ ] Add CSV/JSON escaping for string fields containing special characters

## Phase 13: Persistence Layer Enhancement

### Existing Persistence
- [x] CSV output already implemented in `src/csv_output.hpp/cpp`
- [x] JSON output already implemented in `src/json_output.hpp/cpp`
- [x] System identifier collection in `src/system_info.cpp`
- [x] SQLite3 support enabled via CMake flag `ENABLE_SQLITE_OUTPUT`

### New Command-Line Features Needed

#### Database Commands
- [x] `--list-benchmarks` - IMPLEMENTED: Lists all benchmark runs filtered by system ID, date range, kernel type, sorted by timestamp DESC
- [x] `--search <pattern>` - IMPLEMENTED: Search by system_id, kernel, cpu_model, os_name (LIKE query)
- [x] `--export-db <format> <output>` - IMPLEMENTED: Exports to csv or json
  - [x] `--export-db csv output.csv`
  - [x] `--export-db json output.json`
  - [ ] Proper CSV quoting for fields containing commas/special characters
  - [ ] Proper JSON escaping for string fields (quotes, backslashes)

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
**Note**: Schema is fully implemented in `sqlite_output.cpp::initialize_schema()`. Indexes match spec. `created_at` is set via `strftime('%Y-%m-%d %H:%M:%S', 'now')`.

### Implementation Tasks

#### SQLite Persistence
- [x] Create `src/sqlite_output.hpp` with `SQLiteOutput` class
- [x] Implement `src/sqlite_output.cpp` with:
  - Database initialization and connection management
  - Schema creation (systems + benchmarks tables)
  - Insert benchmark results with system info
  - Query methods for listing/searching
- [x] Create `src/sqlite_input.hpp` with `SQLiteInput` class for reading
- [x] Implement `src/sqlite_input.cpp` with:
  - Database connection and query execution
  - Result set parsing into `BenchmarkResult` structs
  - Filter methods: `query_by_system_id()`, `query_by_kernel()`, `query_by_date_range()`
  - Search: `search(pattern)` with LIKE across multiple fields
  - Export: `export_csv()`, `export_json()`

#### Command-Line Integration
- [x] Add database path option to `Options` struct (default: `~/.mem_band/benchmarks.db`)
- [x] Implement `--list-benchmarks` flag in `main.cpp`
- [x] Implement `--search` flag with pattern matching
- [x] Implement `--export-db` flag for format conversion
- [x] Update `print_usage()` to document new database commands
- [x] Handle SQLite compilation with `#ifdef ENABLE_SQLITE`
- [ ] Document SQLite schema for persistent storage (update specs/ to match actual schema)
- [ ] Add database version/migration column for schema evolution
- [ ] Add `DELETE` command to clear old records

#### SQLite Known Issues
- **Stub values**: `core_count` and `memory_size_mb` in `ensure_system_exists()` are hardcoded to `0`; `platform` is `""` — real `SystemInfo` values never passed
  - **Fix needed**: Pass real `SystemInfo::collect()` values through `run_sqlite_output()` to populate systems table
- **No transaction wrapping**: `append()` does one INSERT at a time; needs `BEGIN/COMMIT` for bulk writes
- **No thread-safety**: No mutex around `sqlite3_stmt` operations for concurrent benchmark runs
- **No JSON escaping**: `export_json()` uses raw string concatenation — quotes/backslashes in `cpu_model` or `system_id` produce malformed JSON
- **No CSV quoting**: `export_csv()` and `csv_output` don't quote fields containing commas

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
- [x] Add `tests/test_sqlite_output.cpp` for SQLite persistence tests (106 lines)
- [x] Add `tests/test_sqlite_input.cpp` for SQLite query tests (153 lines)
- [ ] Add tests for timestamp format consistency (write vs read)
- [ ] Add tests for CSV quoting and JSON escaping edge cases
- [ ] Update `README.md` with new database CLI commands

### Priority Order

1. **High Priority (IMPLEMENTED, bugs remain):**
    - SQLite output class implementation ✅
    - SQLite input class implementation ✅
    - Database schema design ✅
    - `--list-benchmarks` command ✅
    - `--search` command ✅
    - `--export-db` command ✅
    - Populate systems table with real SystemInfo values (core_count, memory_size_mb, platform hardcoded to 0/"")

2. **Medium Priority:**
    - Add transaction wrapping for bulk writes
    - Add thread-safety (mutex) to SQLite I/O
    - Proper CSV/JSON escaping for export functions
    - Database version/migration support
    - Add DELETE/clear-records functionality

3. **Low Priority:**
    - Advanced filtering/sorting
    - Remote storage integration
    - Graph generation from database
    - Database spec document (`specs/database-spec.md`)


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
  - **Current**: System layout is accessed via `info layout` subcommand (not `-L`/`--system-layout` flag)
  - The `Options::system_layout` field exists but is never populated by the argument parser (dead code)
- [x] Support layout output formats:
  - [x] `text` (default) - ASCII diagram with box-drawing characters — `TextFormatter`
  - [x] `mermaid` - Mermaid.js `flowchart TD` — `MermaidFormatter` (has bugs: duplicate CPU node output, hardcoded connections)
  - [x] `json` - Structured layout data — `JSONFormatter` (no metadata escaping)
  - [ ] `--layout-type` flag for manual archetype override (not implemented)
  - [ ] `--update-layout` flag for layout refresh (not implemented)
  - [ ] `--run-benchmark` flag for post-layout execution (not implemented)

### Layout Content
- [x] **CPU Cluster**: CPU model, core count, L3 cache — populated from `SystemInfo::collect()`
- [x] **Memory subsystem**: Total memory (MB), channel count — populated from `SystemInfo::collect()`
- [x] **PCIe devices**: Vendor/device IDs, device type (GPU/NPU/Unknown) — populated from `PlatformDetection::detect()`
  - [ ] PCIe slot/bandwidth information (not collected by platform detection)
  - [ ] Link generation/speed (not collected by platform detection)

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
- [x] Create `src/layout_builder.hpp` with layout data structures
  - [x] `SystemNode` (struct) - CPU/Memory/PCIe nodes with type/name/metadata map
  - [x] `CPUNode` - No separate class; `SystemNode` with type="cpu" plus metadata (model, cores, l3_cache)
  - [x] `MemoryNode` - No separate class; `SystemNode` with type="memory" plus metadata (size_mb, channels)
  - [x] `PCIEDeviceNode` - No separate class; `SystemNode` with type="pcie_device" plus metadata (vendor, device_type)
  - [x] `Connection` (struct) - Link between nodes with type/bandwidth_gb_s
  - [ ] **Connection struct missing from_idx/to_idx** — formatters cannot render correct edges (hardcoded to node0->node1)

#### Phase 13b: Layout Generation
- [x] Implement `src/layout_builder.cpp`:
  - [x] Build CPU cluster from `SystemInfo` (cores, cache)
  - [x] Build memory layout from `SystemInfo` (size, channels)
  - [x] Build PCIe device tree from `PlatformDetection` results
  - [ ] Connect nodes based on interconnect type (NVLink, PCIe, etc.) — **NOT implemented**; connections use hardcoded node indices, not actual interconnect detection

## Phase 13c: Output Formats (COMPLETE)
- [x] Implement text formatter (ASCII box-drawing diagrams) — `TextFormatter`
- [x] Implement Mermaid.js formatter (for documentation) — `MermaidFormatter`
  - [ ] **Bug**: Duplicates CPU node labels (two lines per CPU instead of one)
  - [ ] **Bug**: All connections hardcoded to `node0 --> node1` regardless of actual indices
  - [x ] Connection edge rendering
- [x] Implement JSON formatter (for programmatic use) — `JSONFormatter`
  - [ ] **Bug**: No escaping of metadata strings (quotes/backslashes break JSON)
  - [x] Node metadata serialization
  - [x] Connection serialization (no from/to indices included)

## Phase 13d: Integrated View & Run Mode
- [ ] **`-L, --system-layout` flag** — NOT implemented. Current access is via `info layout` subcommand only.
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
- [x] Layout is wired into `info layout` subcommand in main.cpp (lines 640-680)

## Phase 13e: Runtime Detection Integration
- [ ] Enhance `platform_detection.hpp` with layout-relevant methods:
  - [ ] `GetInterconnectType()` - Returns NVLink/PCIe/InternalBus/None
  - [ ] `GetMemoryTopology()` - Returns unified/shared/discrete
  - [ ] `GetPCIeBandwidth()` - Returns PCIe generation x lanes
- [ ] Auto-detect system archetype from hardware:
  - [ ] Check CPU vendor + GPU vendor combination
  - [ ] Check interconnect presence (NVLink, internal bus)
  - [ ] Map to known archetypes (GB10, Strix, RTX 3090, Mac Studio)

## Phase 13f: Layout Builder Enhancements
- [ ] Add automatic archetype selection based on runtime detection
- [ ] Build layout dynamically from system info + platform detection
- [ ] Support partial layouts (e.g., missing GPU info)
- [ ] Add layout validation (consistency checks for connectivity)

## Phase 13g: Platform Detection Integration
- [ ] Detect interconnect type (NVLink, PCIe, internal bus)
- [ ] Extract PCIe link generation (Gen1-Gen6)
- [ ] Extract memory channel count and bandwidth
- [ ] Handle ARM vs x86 platform differences

## Phase 13h: CLI Integration
- [ ] Add `-L, --system-layout` flag to `Options` struct (currently dead code — field exists but never set by parser)
- [x] Add `--layout-format` subflag for output format selection (text/mermaid/json) — WORKS via `info layout` subcommand
- [ ] Add `--layout-type` manual override flag
- [ ] Add `--update-layout` for layout refresh
- [x] Update `print_usage()` with layout-related options (documented under `info layout`)
- [ ] Support `--run-benchmark` flag for post-layout execution
- [ ] Handle layout display and benchmark execution flow (currently `info layout` does NOT run benchmarks after)

## Phase 13i: Testing
- [x] Add `tests/test_layout_builder.cpp` for unit tests (10 tests)
  - [x] Test layout generation on different hardware configurations
  - [x] Test text, mermaid, and json formatters
  - [x] Test empty layout
  - [ ] Test connection indexing in formatters (currently not tested due to bug)
  - [ ] Fix layout caching behavior (caching not implemented yet)
- [x] Add `tests/test_layout_cli.cpp` for CLI parsing tests (12 tests)
  - [ ] **Note**: Test file defines its own `struct Options` and `parse_args()` — duplicates main.cpp parsing logic. If main.cpp parser changes, the test stays stale.

## Phase 13j: Documentation
- [x] Update README.md with new layout CLI documentation (via `info layout`)
- [ ] Add examples showing layout output for different platforms
  - [x] Document Mermaid output for documentation generation
- [ ] Document system layout workflow (currently only documented under `info` subcommands)
  - [x] Add system archetype identification guide
- [ ] Document known formatter bugs (duplicate CPU nodes in Mermaid, hardcoded connections)

### Priority Order

1. **High Priority:**
    - Layout data structures and core builder class ✅ IMPLEMENTED
    - Text-based ASCII output format ✅ IMPLEMENTED
    - Mermaid.js output ✅ IMPLEMENTED (with bugs to fix)
    - JSON output ✅ IMPLEMENTED (with escaping issues)
    - CLI wired into `info layout` subcommand ✅ IMPLEMENTED
    - Fix Mermaid formatter: duplicate CPU nodes and hardcoded connections
    - Add `from_idx`/`to_idx` to `Connection` struct

2. **Medium Priority:**
    - Runtime environment detection for automatic layout selection
    - Interconnect detection (`GetInterconnectType()`, `GetPCIeBandwidth()`)
    - Automatic archetype detection from hardware
    - Layout caching for performance
    - `--layout-type` manual override
    - Add standalone `-L, --system-layout` flag (in addition to `info layout`)

3. **Low Priority:**
    - Layout comparison/diff functionality
    - Interactive layout exploration tools
    - PCIe link generation detection
    - Advanced layout customization options

### Active ToDos - Platform Detection Enhancement (MARKED COMPLETE)

- [x] Enhance `run_platform_detection()` in `src/main.cpp` to show comprehensive system information - DONE
- [x] Add Architecture field to platform detection output (arm64/x86_64) - DONE
- [x] Add OS name and version to platform detection output - DONE
- [x] Add Memory size (total system memory) to platform detection output - DONE
- [x] Add CPU model details to platform detection output - DONE
- [x] Show Architecture, Platform, RAM size, Core count in platform output - DONE
- [x] Platform identification only shown with `-P` flag or as header before tests - DONE
- [x] Platform header displayed before benchmark execution - DONE
- [ ] Add Windows-specific detection for `detect_cpu_model()`, `detect_memory_size()`, `detect_os()` in system_info.cpp
- [ ] Fix `simd_enabled` hardcoded to `true` — should reflect actual compile-time SIMD availability
- [ ] Fix ARM `detect_cpu_model()` to return actual model (e.g., `sysctl` on macOS, `/proc/cpuinfo` on Linux)

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

## CMake / Build System Issues
- [ ] **Hardcoded paths**: CMakeLists.txt lines 56-59 use `/home/philipp/sqlite_include` and `/home/philipp/sqlite_lib` — should be removed
- [ ] **test_gpu extension mismatch**: CMakeLists.txt references `tests/test_gpu.cpp` but file is `tests/test_gpu.cu` — won't build
- [ ] **test_system_info not registered**: `tests/test_system_info.cpp` exists but has no `add_executable` or `add_test` in CMakeLists.txt
- [ ] **Missing `specs/database-spec.md`**: Referenced in plan but not created
- [x] **Removed redundant `#define ENABLE_SQLITE` from top of sqlite_output.cpp and sqlite_input.cpp** — CMake already defines via `-DENABLE_SQLITE`
- [x] **Fixed CMake bug**: test_sqlite_output and test_sqlite_input now have `target_compile_definitions(... ENABLE_SQLITE)` — tests were silently not running
- [ ] **`output_format` field is dead code**: `Options::output_format` is parsed and validated but `main()` never uses it — benchmark output is always printed as text
- [ ] **`Options::system_layout` field is dead code**: Declared but never set by argument parser

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


