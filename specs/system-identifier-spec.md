# System Identifier Collection and Benchmark Persistence

## Overview

This document specifies the system identifier collection and benchmark persistence mechanisms for the memory bandwidth benchmark. These features enable users to:

1. Collect hardware/system information before running benchmarks
2. Persist benchmark results with system identifiers for later comparison
3. Track performance changes across system updates or configuration changes

## System Identifier

The system identifier consists of the following attributes:

### Required Attributes

| Attribute | Description | Source |
|-----------|-------------|--------|
| `cpu_model` | CPU model name | `/proc/cpuinfo` (Linux), `hw.optional` (macOS), `GetComputerSystemProperties` (Windows) |
| `core_count` | Number of logical cores | `std::thread::hardware_concurrency()` |
| `memory_size_mb` | Total system memory in MB | `/proc/meminfo` (Linux), `sysctl` (macOS), `GlobalMemoryStatusEx` (Windows) |
| `os_name` | Operating system name | Platform APIs |
| `os_version` | Operating system version | Platform APIs |
| `compiler_version` | Compiler used to build | Preprocessor macros, compiler version detection |

### Optional Attributes

| Attribute | Description | Source |
|-----------|-------------|--------|
| `cache_line_size` | L1 cache line size in bytes | `posix_memalign` alignment, CPUID (on x86) |
| `platform` | Target platform identifier | `__unix__`, `_WIN32`, `__powerpc__`, `__i386__` |
| `simd_enabled` | Whether SIMD was enabled | CMake config, `ENABLE_SIMD` flag |
| `benchmark_version` | Version of mem_band | Release version from build system |

### System Identifier Format

The system identifier is stored as a JSON object:

```json
{
  "cpu_model": "AMD Ryzen AI 300 Series",
  "core_count": 12,
  "memory_size_mb": 128000,
  "os_name": "Ubuntu",
  "os_version": "24.04.4 LTS",
  "compiler_version": "gcc 13.2.0",
  "platform": "x86_64-linux",
  "simd_enabled": true,
  "benchmark_version": "1.0.0"
}
```

## Benchmark Persistence

### Storage Format

Benchmark results are persisted in CSV format with the following columns:

| Column | Description |
|--------|-------------|
| `timestamp` | ISO 8601 timestamp when benchmark was run |
| `system_id` | Unique system identifier hash |
| `kernel` | Benchmark kernel name (Copy, Triad, RandomRW, ALU, etc.) |
| `size_mib` | Array size in MiB |
| `data_type` | Data type used (float, double) |
| `iterations` | Number of iterations |
| `bandwidth_gb_s` | Measured bandwidth in GB/s |
| `time_seconds` | Average time per iteration in seconds |
| `bytes_per_iter` | Bytes processed per iteration |

### CSV Example

```csv
timestamp,system_id,kernel,size_mib,data_type,iterations,bandwidth_gb_s,time_seconds,bytes_per_iter
2024-04-08T14:30:00Z,abc123,Copy,256,float,20,3.23,0.62,2097152000
2024-04-08T14:30:00Z,abc123,Triad,256,float,20,3.23,0.93,3145728000
2024-04-08T14:30:00Z,abc123,ALU,256,float,20,4.56,0.34,4194304000
```

### System Identifier Hash

The system identifier hash is a SHA-256 hash of the concatenated system attributes to:

1. Enable benchmark comparison without exposing sensitive hardware details
2. Detect when system hardware changes significantly
3. Group benchmark results by system configuration

## Command-Line Options

### System Identifier Collection (`-R`, `--run-apu`)

Runs only the system identifier collection and exits:

```bash
./mem_band --run-apu
```

**Output:**
```
# System Information
  CPU: AMD Ryzen AI 300 Series
  Cores: 12
  Memory: 124546 MB
  OS: Ubuntu 24.04.4 LTS
  Compiler: gcc 13.2.0
  Platform: x86_64-linux

System ID: abc123def456
```

### Output Format (`-f`, `--output-format`)

Specify the output format for benchmark results:
- `text` (default): Human-readable text output
- `csv`: CSV format for easy parsing
- `json`: JSON format for structured data

```bash
./mem_band --size 256 -f csv
```

### Output File (`-o`, `--output-file`)

Specify a file to append benchmark results (enables persistence):

```bash
./mem_band --size 256 --output-file benchmark-results.csv
```

### Append (`-a`, `--append`)

Force append mode even when no output file is specified (default behavior):

```bash
./mem_band --size 1024 --append
```

## Implementation Details

### File Structure

```
mem_band/
├─ src/
│  ├─ main.cpp            # CLI, orchestration, persistence
│  ├─ system_info.hpp     # System identifier collection
│  ├─ system_info.cpp     # Implementation
│  ├─ csv_output.hpp      # CSV output handling
│  ├─ csv_output.cpp      # Implementation
│  ├─ benchmark.hpp       # Memory kernel implementations
│  └─ aligned_alloc.hpp   # Portable aligned allocation helpers
```

### System Information Collection

The system identifier collection uses platform-specific APIs:

**Linux:**
- `/proc/cpuinfo` for CPU model
- `/proc/meminfo` for memory size
- `/etc/os-release` for OS identification

**macOS:**
- `sysctl` for CPU, memory, hardware info
- `Sysctl` calls for `machdep.cpu`, `hw.memsize`

**Windows:**
- `GetComputerNameEx` for system info
- `GlobalMemoryStatusEx` for memory
- Windows Management Instrumentation (WMI) for CPU info

### Persistence Strategy

1. **On each run**: Append results to the specified output file
2. **System ID**: Compute and store with each run for grouping
3. **Append mode**: Always append, never overwrite (preserves history)
4. **Header handling**: Write CSV headers only on new file creation

## Usage Examples

### Basic Persistence

```bash
./mem_band --size 256 --iters 10 --output-file results.csv
./mem_band --size 512 --iters 10 --output-file results.csv
./mem_band --size 1024 --iters 10 --output-file results.csv
```

**Output (`results.csv`):**
```csv
timestamp,system_id,kernel,size_mib,data_type,iterations,bandwidth_gb_s,time_seconds,bytes_per_iter
2024-04-08T14:30:00Z,abc123,Copy,256,float,10,3.23,0.62,2097152000
2024-04-08T14:30:00Z,abc123,Triad,256,float,10,3.23,0.93,3145728000
...
```

### System Identifier First

```bash
./mem_band --run-apu > system-info.txt
./mem_band --size 256 --output-file results.csv
```

Then compare results across multiple systems:
```bash
grep "kernel: Copy" results.csv
grep "kernel: Triad" results.csv
```

### CSV Analysis

```bash
# Show all Copy results
grep ",Copy," results.csv

# Calculate average bandwidth for Copy at 512 MiB
awk -F',' '$4=="512" && $3=="Copy" {sum+=$7; count++} END {print sum/count}' results.csv

# Group by system ID
awk -F',' '{ids[$2]=1} END {for(id in ids) print id}' results.csv
```

## Error Handling

- **Permission denied**: If the output file cannot be written, print warning and continue with text-only output
- **Invalid format**: If `--output-format` is not supported, default to `text` and warn the user
- **File locked**: Handle concurrent writes gracefully with atomic append operations

## Extensibility

Future enhancements may include:

1. **SQLite backends**: Use SQLite for structured queries and indexing
2. **JSON persistence**: Full JSON output with nested system identifiers
3. **Remote storage**: Upload results to a remote server for collaborative benchmarking
4. **Graph generation**: Generate plots from persistent CSV data
5. **Diff comparison**: Compare two persistent benchmark runs and highlight regressions
