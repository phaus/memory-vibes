# Strategic Benchmark Lab: Accelerator & Memory Architectures

This document outlines the finalized hardware matrix for a robust, cross-platform memory bandwidth benchmarking suite. It covers the four fundamental architectural archetypes of modern compute.

## 1. The Four Architectural Archetypes

Your lab is now structured to test how memory behaves across different physical and logical interconnects.

### A. The Coherent Datacenter Giant: NVIDIA GB10 (DGX Spark)
*   **Architecture:** ARM (Grace) + NVIDIA (Blackwell) via NVLink-C2C.
*   **Memory Model:** Coherent Unified Memory.
*   **Why it's here:** It represents the peak of "tight coupling." Testing here reveals how software behaves when CPU and GPU can see each other's memory with cache coherency and massive interconnect speeds.
*   **Benchmark Focus:** NVLink-C2C vs. local memory throughput; coherency overhead.

### B. The Performance APU: AMD Strix Halo (Ryzen AI Max)
*   **Architecture:** x86 (Zen 5) + RDNA 3.5 Integrated Graphics.
*   **Memory Model:** Shared LPDDR5X (Massive 256-bit or 512-bit bus).
*   **Why it's here:** It is the "x86 counter" to Apple Silicon. It stresses how x86 architectures handle high-performance shared memory contention between a powerful iGPU and CPU.
*   **Benchmark Focus:** CPU-GPU contention; DRAM saturation on shared buses.

### C. The Enthusiast "Evergreen": NVIDIA RTX 3090 (24GB)
*   **Architecture:** x86 Host + Discrete Ampere GPU via PCIe Gen4.
*   **Memory Model:** Discrete VRAM (GDDR6X).
*   **Why it's here:** The gold standard for AI enthusiasts and local developers. With 24GB VRAM and a 384-bit bus (~936 GB/s), it remains the most relevant baseline for discrete GPU workflows.
*   **Benchmark Focus:** PCIe Gen4 saturation; high-speed GDDR6X local effective bandwidth.

### D. The Unified Workstation: Apple Mac Studio (>64GB RAM)
*   **Architecture:** ARM (Apple M-Series Ultra/Max).
*   **Memory Model:** Unified Memory Architecture (UMA).
*   **Why it's here:** It allows testing large-scale LLM inference that exceeds traditional VRAM limits. It offers up to 800 GB/s bandwidth available to both CPU and GPU.
*   **Benchmark Focus:** Zero-copy efficiency; Apple Silicon proprietary fabric scaling.

---

## 2. Hardware Comparison Matrix

| Platform | Arch Type | Interconnect | Memory Type | Bandwidth (Target) |
| :--- | :--- | :--- | :--- | :--- |
| **GB10** | Coherent ARM | NVLink-C2C | LPDDR5X (Unified) | > 1 TB/s (Local) |
| **Strix Halo** | Shared x86 | Internal Bus | LPDDR5X (Shared) | ~500 GB/s |
| **RTX 3090** | Discrete x86 | PCIe Gen4 | GDDR6X (Local) | ~936 GB/s |
| **Mac Studio**| Unified ARM | UltraFusion | LPDDR5 (Unified) | ~800 GB/s |

---

## 3. Methodological Coverage

By using this specific assortment, your benchmark tool is forced to handle four distinct "data movement" philosophies:

1.  **Explicit Copy (RTX 3090):** Data must be moved over PCIe. Methodology: `cudaMemcpy`.
2.  **Shared-DRAM Contention (Strix Halo):** Data stays in RAM, but CPU and GPU fight for cycles. Methodology: `ROCm` / `Unified Memory`.
3.  **Unified Fabric (Mac Studio):** Truly unified. No copying, just pointer passing. Methodology: `Metal` / `Storage Buffer`.
4.  **Coherent High-Speed Link (GB10):** Hardware-level coherency. Methodology: `NVLink` / `BusGrind`.

---

## 4. Summary of Lab Value

| Capability | Provided By |
| :--- | :--- |
| **x86 Ecosystem** | Strix Halo, RTX 3090 (Host) |
| **ARM Ecosystem** | GB10, Mac Studio |
| **AI Community Vibe** | RTX 3090, Mac Studio |
| **Enterprise Future** | GB10 |
| **High-End Desktop** | Mac Studio, Strix Halo |

---

## 5. Next Steps for Implementation

1.  **Develop on RTX 3090**: Establish your "Discrete Baseline."
2.  **Verify on Strix Halo**: Adjust methodology for "Shared Memory" (ensure your tool doesn't try to "copy" where it isn't needed).
3.  **Optimize for Mac Studio/GB10**: Refine for ARM-based unified/coherent logic.
4.  **Final Stress Test**: Run simultaneous benchmarks on Strix Halo to measure how CPU activity degrades GPU memory performance.

---

This spec complements the [benchmark specification](./benchmark-spec.md) by providing guidance on hardware diversity for comprehensive memory-bandwidth testing across different memory architectures.
