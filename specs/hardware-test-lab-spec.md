# Recommended Hardware Test Lab for Memory Bandwidth Benchmarking

This document proposes a practical hardware assortment for building a reliable memory-bandwidth benchmark lab across CPUs, APUs/iGPUs, and discrete GPUs.

The goal is not to use the newest hardware, but to cover the most important platform classes with mature, affordable, and still relevant devices.

## 1. Goals of the Test Lab

A good benchmark lab should cover the main memory models that influence measurement methodology:

1. **Discrete GPU over PCIe**
   - Host ↔ Device transfer bandwidth
   - Device-local memory bandwidth
2. **Integrated GPU / APU with shared DRAM**
   - CPU and GPU share system memory
   - No true PCIe transfer path between CPU and GPU
3. **ARM-based shared-memory systems**
   - Unified memory / shared DRAM behavior
   - Different CPU architecture and tooling
4. **Commonly deployed hardware families**
   - Prefer representative mainstream systems over rare flagship parts

## 2. Why Use Steam Hardware Survey as One Input

The Steam Hardware Survey is useful as a rough proxy for widely deployed client GPUs and iGPUs, especially for consumer and workstation-adjacent systems.

Useful references:
- [Steam Hardware Survey](https://store.steampowered.com/hwsurvey)
- [Steam Video Card Statistics](https://store.steampowered.com/hwsurvey/videocard)
- [Steam Video Card Popularity](https://store.steampowered.com/hwsurvey/videocard/?sort=pop)

Important limitation:
- Steam does **not** directly report NPUs or AI accelerator blocks.
- It is still useful to estimate the prevalence of GPU families that include AI-capable hardware such as NVIDIA Tensor Cores, Intel XMX, or AMD matrix instructions.

## 3. Recommended Platform Categories

A reliable lab should ideally include the following categories:

### A. x86 + NVIDIA Discrete GPU
Representative of the most common AI-capable consumer/workstation accelerator class.

**Suggested devices:**
- **NVIDIA GeForce RTX 3060 12GB**
- **NVIDIA GeForce RTX 4060**
- Optional older baseline: **NVIDIA GeForce RTX 2060**

**Reason:**
- Strong real-world relevance
- Mature CUDA tooling
- Excellent baseline for PCIe transfer and device-local bandwidth measurement

### B. x86 + AMD Discrete GPU
Needed for cross-vendor comparison of discrete GPU behavior.

**Suggested devices:**
- **AMD Radeon RX 6600**
- **AMD Radeon RX 6700 XT**
- Optional older baseline: **AMD Radeon RX 5700 XT**

**Reason:**
- Good mainstream AMD coverage
- Mature enough driver/tooling stack
- Useful contrast to NVIDIA on PCIe and local memory bandwidth

### C. x86 Integrated GPU / APU
Required to validate shared-memory systems where CPU and GPU use the same DRAM.

**Suggested devices:**
- **AMD Ryzen 7 7840U / 8840U with Radeon 780M**
- **Intel Core Ultra 7 155H class system**
- Budget fallback: older **Intel Iris Xe** laptop

**Reason:**
- Important for APU/iGPU methodology
- Shared-memory systems behave differently from discrete GPUs
- Increasingly relevant in client and mobile systems

### D. Intel Discrete GPU
Optional but strongly recommended for a complete cross-vendor comparison.

**Suggested devices:**
- **Intel Arc A770**
- **Intel Arc A750**

**Reason:**
- Different software stack and memory behavior
- Useful for validating vendor-neutral methodology

### E. ARM-based Shared-Memory Platform
Important for non-x86 coverage and unified-memory systems.

**Suggested devices:**
- **Apple M1 or M2**
- **Snapdragon X Elite**
- Optional server-side ARM CPU platform: **Ampere Altra**

**Reason:**
- Covers ARM memory hierarchy and unified/shared memory
- Important for architecture-aware benchmarking

## 4. Recommended Lab Sizes

### 4.1 Minimum Serious Setup
This is the smallest setup that still gives strong methodological coverage.

**Hardware:**
1. **x86 + NVIDIA RTX 3060**
2. **x86 + AMD RX 6600 or RX 6700 XT**
3. **AMD APU system with Radeon 780M**
4. **Intel iGPU / Core Ultra system**
5. **ARM shared-memory platform**  
   - Apple M1/M2 or Snapdragon X Elite

**Coverage:**
- NVIDIA discrete GPU
- AMD discrete GPU
- x86 shared-memory APU/iGPU
- ARM shared-memory platform

### 4.2 Recommended Balanced Setup
A stronger setup for benchmarking across all major client-relevant architecture classes.

**Hardware:**
1. **Desktop x86 + RTX 3060**
2. **Desktop x86 + RTX 4060**
3. **Desktop x86 + RX 6700 XT**
4. **Mini-PC or laptop with Ryzen 7840U / 8840U (Radeon 780M)**
5. **Laptop with Intel Core Ultra**
6. **Desktop x86 + Intel Arc A770**
7. **Apple M1/M2** or **Snapdragon X Elite**

**Coverage:**
- Mainstream NVIDIA discrete class
- Mainstream AMD discrete class
- Intel discrete GPU
- AMD APU / shared DRAM
- Intel iGPU / shared DRAM
- ARM shared-memory systems

### 4.3 Comprehensive but Still Practical Setup
Useful if both client and inference-adjacent deployment matter.

**Hardware:**
1. **RTX 3060**
2. **RTX 4060**
3. **RX 6600**
4. **RX 6700 XT**
5. **Intel Arc A770**
6. **AMD Ryzen 7840U / 780M**
7. **Intel Core Ultra system**
8. **Apple M1/M2**
9. Optional inference-oriented older datacenter card: **NVIDIA T4**
10. Optional ARM server CPU: **Ampere Altra**

## 5. Why These Devices Are Good Choices

### NVIDIA RTX 3060
- Very common and highly representative of mainstream AI-capable client GPUs
- Includes Tensor Cores
- Mature drivers and broad software support
- Good used-market availability

### NVIDIA RTX 4060
- Represents newer mainstream deployment
- Useful contrast to Ampere-era RTX 3060
- Good power efficiency

### AMD RX 6600 / 6700 XT
- Good mainstream AMD dGPU representatives
- Affordable and mature
- Useful for PCIe + local VRAM bandwidth comparisons

### AMD Ryzen 7840U / 8840U with Radeon 780M
- Strong modern x86 APU example
- Shared-memory design makes it ideal for APU methodology validation
- Relevant for portable systems

### Intel Core Ultra
- Important modern Intel integrated-graphics platform
- Useful to compare with AMD APUs
- Relevant for iGPU/shared-memory and AI-era client systems

### Intel Arc A770
- Gives Intel discrete GPU coverage
- Useful for broader vendor-neutral validation
- Distinct memory and tooling stack

### Apple M1 / M2
- Highly relevant ARM unified-memory platform
- Strong contrast to x86 + PCIe dGPU systems
- Very useful for testing shared-memory assumptions

### Snapdragon X Elite
- Relevant if Windows-on-ARM or ARM laptops matter
- Helps cover modern ARM client systems outside Apple

### NVIDIA T4
- Older, widely deployed inference accelerator
- Often available cheaply on the used market
- Useful if the benchmark lab should also reflect deployed AI inference hardware

## 6. What Not to Prioritize First

The following classes are useful later, but not ideal as the first systems in a practical benchmark lab:

- **NVIDIA H100 / H200**
- **AMD MI300**
- **Grace Hopper / GH200**
- **Very old pre-RTX gaming GPUs**
- **Rare embedded boards as primary targets**

**Reason:**
- Expensive or difficult to acquire
- May not represent mainstream deployment patterns
- Tooling and software support may be complex for basic benchmarking

---

This spec complements the [benchmark specification](./benchmark-spec.md) by providing guidance on hardware diversity for comprehensive memory-bandwidth testing across different memory architectures.
