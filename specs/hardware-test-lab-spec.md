# Strategic Benchmark Lab: Accelerator & Memory Architectures

This document outlines the finalized hardware matrix for a robust, cross-platform memory bandwidth benchmarking suite. It covers four distinct architectural approaches to modern high-performance compute.

## 1. Hardware Inventory & Architectural Archetypes

The lab is structured to test memory behavior across different interconnects, from integrated shared-memory SoCs to workstation-class discrete accelerators.

### A. The Reference AI Workstation: NVIDIA DGX Spark
*   **Role:** Personal AI Supercomputer / Workstation Platform.
*   **Why it's here:** As a dedicated, integrated NVIDIA solution for local AI development, it serves as the primary NVIDIA reference platform. It represents the "appliance" approach to AI hardware, where performance is tuned for local training and inference.
*   **Benchmark Focus:** Effective accelerator memory throughput; host-to-device data movement efficiency; performance baseline for professional NVIDIA AI stacks.

### B. The Performance APU: AMD Strix Halo (Ryzen AI Max)
*   **Role:** High-Performance x86 APU (Shared Memory).
*   **Why it's here:** It is the direct x86 competitor to high-end unified architectures. It stresses how x86 systems handle massive shared DRAM bandwidth (via wide LPDDR5X interfaces) between a powerful iGPU and CPU.
*   **Benchmark Focus:** CPU-GPU memory contention; DRAM saturation on shared buses; efficiency of shared-memory pointer passing.

### C. The Enthusiast Milestone: NVIDIA RTX 3090 (24GB)
*   **Role:** Discrete Workstation GPU (PCIe-based).
*   **Why it's here:** The "evergreen" of the AI community. With 24GB VRAM and a 384-bit bus (~936 GB/s), it represents the reality of millions of local AI developers and is the gold standard for comparing discrete PCIe-based workflows.
*   **Benchmark Focus:** PCIe Gen4 saturation; high-speed GDDR6X local effective bandwidth; legacy support for large weights in discrete VRAM.

### D. The Unified ARM Workstation: Apple Mac Studio (>64GB RAM)
*   **Role:** ARM-based Unified Memory Architecture (UMA).
*   **Why it's here:** It allows testing large-scale LLM inference that exceeds traditional discrete VRAM limits by using a common pool of high-speed system memory for both CPU and GPU.
*   **Benchmark Focus:** Zero-copy efficiency (Metal); Apple Silicon internal fabric scaling; memory behavior in massive-scale unified RAM environments.

---

## 2. Hardware Comparison Matrix

| Platform | Category | Arch Type | Memory Model | Key Metric |
| :--- | :--- | :--- | :--- | :--- |
| **DGX Spark** | AI Workstation | NVIDIA / Integrated | Professional Stack | Reference AI Perf |
| **Strix Halo** | Client APU | x86 (AMD) | Shared LPDDR5X | Bus Contention |
| **RTX 3090** | Discrete GPU | x86 + PCIe | Dedicated GDDR6X | PCIe Transfer |
| **Mac Studio**| Unified ARM | ARM (Apple) | Unified LPDDR5 | Zero-Copy Effect |

---

## 3. Methodological Coverage

This assortment forces the benchmarking tool to handle the four fundamental data movement paradigms:

1.  **Workstation Integrity (DGX Spark):** Optimized professional driver stack and integrated hardware behavior.
2.  **Explicit PCIe Copy (RTX 3090):** Physical data movement over the PCIe bus (Host ↔ Device).
3.  **Shared-DRAM Latency (Strix Halo):** Contentious access to the same physical DRAM modules from different processors (x86).
4.  **True Unified Access (Mac Studio):** No physical transfer; logic focuses on memory ownership and fabric hits (ARM).

---

## 4. Lab Strategic Value

| Goal | Platform |
| :--- | :--- |
| **NVIDIA Primary Reference** | DGX Spark |
| **Mainstream Baseline** | RTX 3090 |
| **Future Mobile/iGPU Tech** | Strix Halo |
| **Large Model (LLM) Inference** | Mac Studio (>64GB) |
| **Cross-Arch Validation** | x86 (AMD/Intel) vs. ARM (Apple/Grace) |

---

## 5. Summary & Next Steps

This lab setup provides 100% coverage for the memory models you are likely to encounter in modern software architecture.

1.  **Establish Baselines**: Use the RTX 3090 for discrete PCIe and the DGX Spark for professional NVIDIA performance metrics.
2.  **Shared Memory Stress**: Use Strix Halo to test how your tool handles "blind" shared memory where the physical bus is shared.
3.  **Unified Optimization**: Use the Mac Studio to ensure the tool recognizes when *not* to copy data at all.
4.  **Platform Inventory**: Use your C-based inventory logic to ensure the correct benchmark is automatically triggered for each of these distinct platforms. 

---

This spec complements the [benchmark specification](./benchmark-spec.md) by providing guidance on hardware diversity for comprehensive memory-bandwidth testing across different memory architectures.
