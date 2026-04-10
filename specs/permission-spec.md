# Permission Requirements Specification

## Overview

This document describes the permission requirements for running the memory bandwidth benchmark and its optional features on Linux, macOS, and Windows.

---

## Core Benchmark (No Extra Permissions)

The basic memory bandwidth benchmark (`mem_band`) requires **no special permissions**:

- Runs with standard user privileges
- No root/administrator access required
- Uses only C++17 standard library APIs

```bash
# Basic usage - no permissions needed
./mem_band --size 256 --iters 20
```

---

## Platform Detection Permissions

### Linux: `/sys` Filesystem Access

**Purpose**: PCIe hardware enumeration for platform detection via `--show-platform` and `--show-features` flags

**Access pattern**:
- Read-only access to `/sys/bus/pci/devices`
- Uses standard POSIX `opendir()`, `readdir()`, `fopen()` calls
- No special kernel modules required

**Default permissions**:
```bash
# Standard user can read basic PCI info
ls /sys/bus/pci/devices/0000:01:00.0/
# vendor  device  class  ...
```

**Group membership for comprehensive access** (optional, recommended):
```bash
# Add user to video group for GPU device access
sudo usermod -aG video $USER

# Add user to dmi group for system information
sudo usermod -aG dmi $USER

# Or useplugdev group (common on Ubuntu/Debian)
sudo usermod -aG plugdev $USER

# Apply changes (logout/login required)
newgrp video
```

**Permission check code**:
```cpp
#ifdef __linux__
    DIR* dir = opendir("/sys/bus/pci/devices");
    if (!dir) {
        std::cerr << "Warning: Cannot access /sys/bus/pci/devices\n";
        std::cerr << "Try: sudo usermod -aG video $USER\n";
        // Fallback to compile-time CPU vendor detection
    }
#endif
```

**Fallback behavior**:
- If `/sys` access fails, uses compile-time CPU vendor detection
- Platform detection returns partial results
- No build-time dependency required

---

### Windows: WMI Access

**Purpose**: Hardware inventory via Windows Management Instrumentation

**Access pattern**:
- Uses COM/ActiveX APIs
- Standard WMI queries to `root\cimv2`
- No kernel-mode drivers required

**Default permissions**:
- Standard user privileges sufficient for WMI queries
- No administrator rights required

**Possible issues and solutions**:
```powershell
# Check WMI service status
Get-Service winmgmt

# If execution policy blocks WMI scripting:
Set-ExecutionPolicy RemoteSigned -Scope CurrentUser

# Test WMI access
Get-WmiObject -Class Win32_VideoController
```

**Fallback behavior**:
- If WMI queries fail, uses CPUID-based vendor detection
- Platform detection continues with available data
- No build-time dependency required

---

### macOS: CoreFoundation / sysctl

**Purpose**: System information via CoreFoundation framework

**Access pattern**:
- Uses `sysctl()` system calls
- CoreFoundation APIs for hardware enumeration
- No privileged access required

**Default permissions**:
- No special permissions required
- Standard user access to sysctl interface

**Fallback behavior**:
- If sysctl calls fail, uses compiler macros
- System information collection returns partial results

---

## Optional Features Permissions

### CUDA (NVIDIA GPU Benchmarking)

**Runtime library loading**:
```bash
# Check if CUDA libraries are accessible
ldd mem_band | grep cuda
```

**Permissions**:
- Read access to library search paths
- Standard user privileges for `dlopen()`
- No root required

**GPU device access**:
- CUDA runtime requires GPU driver installed
- No special permissions beyond driver access
- Standard user can access CUDA devices

---

### ROCm (AMD GPU/NPU Benchmarking)

**Runtime library loading**:
```bash
# Check if ROCm libraries are accessible
ldd mem_band | grep hip
```

**Permissions**:
- Standard user privileges for `dlopen()`
- No root required for library loading

**GPU device access**:
- ROCm driver must be installed
- User may need `video` group membership:
```bash
sudo usermod -aG video $USER
```

---

### SSD I/O Benchmark

**Filesystem access**:
- Required: Write access to benchmark directory
- Required: Read access for validation

**Default permissions**:
```bash
# SSD benchmark uses /tmp by default
# Standard user can write to /tmp

# Custom path requires write permission
./mem_band --ssd --ssd-path /home/user/bench
```

**Permissions check**:
```cpp
// Before SSD benchmark, verify directory is writable
std::ofstream test_file(path + "/.write_test");
if (!test_file.is_open()) {
    std::cerr << "Error: Cannot write to SSD benchmark directory: " << path;
    return;
}
```

---

## Permission Troubleshooting Guide

### Linux

**Problem**: "Cannot access /sys/bus/pci/devices"

**Solution**:
```bash
# Check current group membership
groups $USER

# Add to required groups
sudo usermod -aG video,dmi,plugdev $USER

# Apply changes
newgrp video
# Or logout and login
```

**Verify fix**:
```bash
ls -la /sys/bus/pci/devices/ | head -5
```

---

### Windows

**Problem**: WMI queries return empty results

**Solutions**:
1. Check WMI service:
```powershell
Get-Service winmgmt
# Should show "Running"
```

2. Adjust execution policy (if scripting):
```powershell
Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
```

3. Run WMI diagnostics:
```powershell
# Test WMI connectivity
Get-WmiObject -Class Win32_VideoController
```

---

### macOS

**Problem**: sysctl calls fail

**Solutions**:
1. Check System Integrity Protection (SIP):
```bash
csrutil status
```

2. Verify sysctl access:
```bash
sysctl -n hw.memsize
```

---

## Security Considerations

### Principle of Least Privilege

- **Default**: No special permissions required
- **Optional**: Group membership for hardware access only
- **Never**: Root/administrator access required

### Sandboxing

- Benchmark can run in containers (Docker/Podman)
- SSD benchmark requires volume mount with write permissions
- Platform detection may be limited in containers

### Audit Trail

No logging of sensitive data:
- No passwords or keys collected
- System identifiers are hardware metadata only
- Output (text, CSV, JSON) contains benchmark metrics only

---

## Build-Time Permissions

No special permissions required for building:

```bash
# Standard user can build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Optional features:
- CUDA/ROCm detection at build time requires toolkits installed
- Standard user can install toolkits to `$HOME`
- No root required for development builds

---

## Summary Matrix

| Feature | Platform | Required Permissions | Standard User | Root Needed |
|---------|----------|---------------------|---------------|-------------|
| Core benchmark | Linux/macOS/Windows | None | ✅ | ❌ |
| Platform detection | Linux | `/sys` read-only | ✅ | ❌ (optional groups) |
| Platform detection | Windows | WMI queries | ✅ | ❌ |
| Platform detection | macOS | sysctl | ✅ | ❌ |
| CUDA benchmark | Linux/macOS/Windows | Library load | ✅ | ❌ |
| ROCm benchmark | Linux | Library load | ✅ | ❌ (optional video group) |
| SSD benchmark | Linux/macOS/Windows | Directory write | ✅ | ❌ |
| Unit tests | Linux/macOS/Windows | Build + runtime | ✅ | ❌ |

**Legend**:
- ✅ = Requires only standard user privileges
- ❌ = Root/administrator NOT required

---

## Testing Permission Requirements

### Quick Permission Test

```bash
# Test core benchmark (no permissions)
./mem_band --quick-test

# Test platform detection (Linux)
./mem_band --show-platform

# Test SSD (requires write to /tmp)
./mem_band --ssd --quick-test
```

### Permission Violation Handling

If permissions are insufficient:
1. Print clear error message
2. Suggest fix (group membership, execution policy, etc.)
3. Gracefully degrade to available functionality
4. Exit with non-zero status code for CI/CD

---

*Generated by the OpenCode assistant.*
