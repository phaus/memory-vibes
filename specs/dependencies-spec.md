# Optional Dependencies Specification

## Overview

This document defines the optional runtime and build-time dependencies for the memory bandwidth benchmark. The core benchmark functionality requires **no external dependencies** - only the C++17 standard library. Optional features can be enabled via CMake options, allowing the base program to run without any additional libraries.

## Core Design Philosophy

1. **Zero dependencies by default**: The base `mem_band` executable runs with only the C++17 standard library
2. **Opt-in dependencies**: All additional features are enabled via CMake `option()` flags
3. **Fallback mechanisms**: When optional dependencies are unavailable, the code gracefully degrades to core functionality
4. **Test isolation**: Tests that require optional dependencies are only built when those dependencies are enabled

---

## Dependency Categories

### 1. Build-Time Dependencies

#### GoogleTest (Testing Framework)
- **Purpose**: Unit testing framework
- **Status**: Currently included via FetchContent (build-only)
- **Impact**: Only affects test executables, not the main `mem_band` binary
- **Option**: Always enabled for test builds, but tests can be disabled with `-DBUILD_TESTING=OFF`

```cmake
option(BUILD_TESTING "Build test executables" ON)
```

#### SIMD Compiler Flags
- **Purpose**: Enable AVX2/SSNE2/Altivec vectorization
- **Status**: Optional, disabled by default
- **Impact**: Performance improvement for core kernels
- **Option**: `-DENABLE_SIMD=ON`

```cmake
option(ENABLE_SIMD "Enable SIMD vectorized kernels" OFF)
```

---

### 2. Runtime Dependencies (Optional)

#### CUDA Toolkit (GPU Benchmarking)
- **Purpose**: NVIDIA GPU memory bandwidth measurement
- **Header**: `cuda_runtime.h`
- **Library**: `cuda`, `nvinfer` (if using TensorRT)
- **Impact**: Enables `src/gpu_benchmark.hpp` functionality
- **Detection**: CMake `find_package(CUDA)` or `find_package(CUDAToolkit)`

**Fallback**: If CUDA is not found:
- GPU benchmark functions return error codes
- Main program continues with CPU-only benchmarks
- Warning message displayed: "CUDA not available - GPU benchmarking disabled"

**Integration point**:
```cpp
// In gpu_benchmark.hpp
#ifdef ENABLE_CUDA
#include <cuda_runtime.h>
// CUDA-enabled implementation
#else
// Stub implementation that returns error
#endif
```

#### ROCm Library (AMD GPU/NPU Benchmarking)
- **Purpose**: AMD GPU and NPU benchmarking
- **Header**: `hip/hip_runtime.h`, `rocm_smi/rocm_smi.h`
- **Library**: `hipblas`, `rocm_smi`
- **Impact**: Enables AMD GPU/NPU specific benchmark tests
- **Detection**: Detect ROCM_PATH environment variable or CMake `find_library`

**Fallback**: Same as CUDA - graceful degradation

#### Linux `/sys` Filesystem Access
- **Purpose**: Hardware inventory PCIe scanning
- **Status**: Linux-only, requires root or group permissions
- **Impact**: `--run-apu` system identifier collection
- **Detection**: Compile-time `__linux__` macro
- **No library needed**: Uses standard POSIX `opendir`, `readdir`

**Permission requirement**: Users may need to be in the `video` or `dmi` group:
```bash
sudo usermod -aG video $USER
sudo usermod -aG dmi $USER
```

#### Windows WMI Access
- **Purpose**: Hardware inventory on Windows
- **Status**: Windows-only, uses COM/ActiveX
- **Impact**: `--run-apu` system identifier on Windows
- **Library**: `combase.lib`, `wbemuuid.lib`
- **No external dependency**: Uses Windows SDK

#### macOS CoreFoundation
- **Purpose**: System information on macOS
- **Status**: macOS-only
- **Impact**: `--run-apu` on macOS
- **Library**: `CoreFoundation.framework`
- **No external dependency**: Uses macOS SDK

---

## CMake Integration Strategy

### Modular Build System

The CMakeLists.txt should be restructured to support optional dependencies:

```cmake
# Core executable (always built, no optional dependencies)
add_executable(mem_band src/main.cpp src/system_info.cpp src/csv_output.cpp)

# Optional: GPU benchmark executable (requires CUDA)
option(ENABLE_CUDA "Enable NVIDIA GPU benchmarking" OFF)
if(ENABLE_CUDA)
    find_package(CUDAToolkit REQUIRED)
    add_executable(mem_band_gpu src/main.cpp src/gpu_benchmark.cpp src/system_info.cpp)
    target_link_libraries(mem_band_gpu CUDA::cudart CUDA::cuda_driver)
endif()

# Optional: AMD benchmark executable (requires ROCm)
option(ENABLE_ROCM "Enable AMD GPU/NPU benchmarking" OFF)
if(ENABLE_ROCM)
    find_path(ROCM_PATH include/hip/hip_runtime.h)
    if(ROCM_PATH)
        add_executable(mem_band_amd src/main.cpp src/gpu_benchmark.cpp)
        target_include_directories(mem_band_amd PRIVATE ${ROCM_PATH}/include)
        target_link_libraries(mem_band_amd hip)
    endif()
endif()

# Test executables (require GoogleTest)
option(BUILD_TESTING "Build test executables" ON)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    add_executable(test_benchmark tests/test_benchmark.cpp)
    target_link_libraries(test_benchmark GTest::gtest_main)
    
    # Only build GPU tests if CUDA is enabled
    if(ENABLE_CUDA)
        add_executable(test_gpu tests/test_gpu.cpp)
        target_link_libraries(test_gpu GTest::gtest_main CUDA::cudart)
    endif()
endif()
```

### Feature Detection Pattern

Use CMake `.check_cxx_source_compiles` for optional features:

```cmake
# Check for CUDA support
option(CHECK_CUDA "Check if CUDA compilation is possible" ON)
if(CHECK_CUDA)
    include(CheckIncludeFileCXX)
    check_include_file_cxx(cuda_runtime.h CUDA_FOUND)
    if(CUDA_FOUND)
        set(ENABLE_CUDA ON CACHE BOOL "" FORCE)
    endif()
endif()
```

---

## Platform-Specific Dependencies

### Linux
**Core dependencies**: None (C++17 stdlib only)

**Optional**:
- `/sys` filesystem (standard kernel interface - no library)
- Performance Monitoring Units (PMU) - requires kernel modules

```bash
# Check if /sys is accessible
if(EXISTS /sys/bus/pci/devices)
    set(SYS_PCI_AVAILABLE ON)
endif()
```

### Windows
**Core dependencies**: None (C++17 stdlib only)

**Optional**:
- WMI access (Windows SDK - already included)
- DMA buffers for memory bandwidth (kernel-level access)

### macOS
**Core dependencies**: None (C++17 stdlib only)

**Optional**:
- IOKit for low-level hardware access (optional, uses sysctls instead)
- Power management interfaces (`IOKit/power_src/IOPowerSources.h`)

---

## Dependency Matrix

| Feature | Core | CUDA | ROCm | Linux /sys | Windows WMI | macOS IOKit |
|---------|------|------|------|------------|-------------|-------------|
| Memory bandwidth (Copy/Triad) | ✅ | - | - | - | - | - |
| SSD I/O benchmark | ✅ | - | - | ✅ | ✅ | ✅ |
| APU system identifier | ✅ | - | - | ✅ | ✅ | ✅ |
| ALU benchmarks | ✅ | - | - | - | - | - |
| NPU benchmarks | ✅ (mock) | - | ✅ | - | - | - |
| GPU memory bandwidth | - | ✅ | ✅ | - | - | - |
| Unit tests | ✅ (GTest) | ✅ | - | - | - | - |

**Legend**:
- ✅ = Required
- - = Not applicable
- (mock) = Mock implementation without hardware

---

## Development Workflow

### Default Build (No Dependencies)
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```
**Result**: Core benchmark only, no GPU/ROCm support

### Full Build (All Optional Features)
```bash
mkdir build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_SIMD=ON \
  -DENABLE_CUDA=ON \
  -DENABLE_ROCM=ON \
  -DBUILD_TESTING=ON
cmake --build .
```

### Minimal Build (Base Only)
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build .
```

---

## Runtime Behavior

### Graceful Degradation

When optional dependencies are unavailable:

1. **Warning messages**: Display non-fatal warnings about unavailable features
2. **Disabled commands**: `--run-gpu`, `--run-npu` commands show "not available" message
3. **Core functionality**: Memory bandwidth benchmarks always work

```cpp
void run_npu_benchmark(const Options& opts) {
    #if defined(ENABLE_ROCM)
        mem_band::NPUConfig config;
        auto result = mem_band::mock_npu_benchmark(config);
        mem_band::print_npu_result(result, config);
    #else
        std::cerr << "NPU benchmarking requires ENABLE_ROCM=ON at build time\n";
        std::cerr << "Rebuild with: cmake .. -DENABLE_ROCM=ON\n";
        return;
    #endif
}
```

### Dependency Discovery at Runtime

Use `dlopen`/`LoadLibrary` for truly optional runtime dependencies:

```cpp
// Example: Load CUDA dynamically
void* cuda_handle = dlopen("libcuda.so", RTLD_LAZY);
if (cuda_handle) {
    // CUDA is available, load functions
    typedef cudaError_t (*cudaMalloc_t)(void**, size_t);
    cudaMalloc = (cudaMalloc_t)dlsym(cuda_handle, "cudaMalloc");
    cuda_available = true;
} else {
    std::cerr << "CUDA library not found - GPU benchmarking disabled\n";
}
```

---

## Recommendations

1. **Keep core dependency-free**: No external libraries for `mem_band` binary
2. **Use FetchContent for dev dependencies**: GoogleTest via FetchContent is acceptable
3. **Provide clear build instructions**: Document how to enable optional features
4. **Use CMake options consistently**: `-DENABLE_XXX=ON/OFF` pattern
5. **Document permissions**: Linux `/sys` access may require group membership

---

## Permission Requirements

### Linux /sys Filesystem Access

**Purpose**: PCIe hardware enumeration for platform detection

**Required permissions**:
- Standard POSIX file system access (read-only)
- No root required for basic `/sys/bus/pci/devices` scanning
- Some PCIe devices may require elevated privileges for full enumeration

**Group membership** (optional, for comprehensive access):
```bash
# Add user to video group for GPU device access
sudo usermod -aG video $USER

# Add user to dmi group for system information
sudo usermod -aG dmi $USER
```

**Fallback**: If `/sys` access fails:
- Platform detection uses compile-time CPU vendor detection
- System information collection returns partial results
- No build-time dependency required

### Windows WMI Access

**Purpose**: Hardware inventory via Windows Management Instrumentation

**Required permissions**:
- Standard user privileges sufficient for WMI queries
- No administrator rights required
- May require PowerShell execution policy adjustment

**Execution policy** (if needed):
```powershell
Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
```

**Fallback**: If WMI queries fail:
- Platform detection uses CPUID-based vendor detection
- System information collection continues with available data

### macOS CoreFoundation

**Purpose**: System information via CoreFoundation and sysctl

**Required permissions**:
- No special permissions required
- Standard user access to sysctl interface

**Fallback**: If sysctl calls fail:
- Platform detection uses compiler macros
- System information collection returns partial results

### Runtime Dynamic Library Loading

**Purpose**: Detect optional dependencies at runtime (CUDA, ROCm, JSON, SQLite)

**Required permissions**:
- Read access to library search paths
- Dlopen/LoadLibrary system calls (standard user privileges)

**Library search paths** (default):
```bash
# Linux
/usr/lib:/usr/lib64:/usr/local/lib:$(LD_LIBRARY_PATH)

# Windows
C:\Windows\System32:$(PATH)
```

**Permissions needed**:
- No root/admin required
- No special security context needed
- Standard executable permissions sufficient

---

## Future Considerations

1. **JSON serialization library** (rapidjson, nlohmann/json) - optional output format
2. **SQLite3** - structured benchmark storage
3. **Boost.Process** - better process management (already uses std::process)
4. **fmt** - better string formatting (already uses std::ostringstream)

These can all be added as optional dependencies in the future without affecting core functionality.

---

---
