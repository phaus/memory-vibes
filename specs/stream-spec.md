# STREAM Benchmark Specification

## Overview
The **STREAM** benchmark measures the sustainable memory bandwidth of a system using a set of simple vector kernels that operate on large, contiguous arrays.  It is designed to be memory‑bound, so the reported bandwidth reflects the limits of the main memory subsystem rather than CPU compute capability.

## Supported Kernels
| Kernel | Operation | Bytes per element | FLOPs per element |
|--------|-----------|-------------------|-------------------|
| **Copy** | `c[i] = a[i]` | 2 × size_of(T) (read + write) | 0 |
| **Scale** | `b[i] = q * a[i]` | 2 × size_of(T) (read + write) | 1 |
| **Add** | `c[i] = a[i] + b[i]` | 3 × size_of(T) (two reads + write) | 1 |
| **Triad** | `c[i] = a[i] + q * b[i]` | 3 × size_of(T) (two reads + write) | 2 |

*`q` is a scalar of the same type as the arrays.*

## Run Rules (summary)
1. **Array size** – each array must be at least **4× the total size of all last‑level caches** or **1 000 000 elements**, whichever is larger.  The default size used by the reference implementation is **2 000 000 elements** (≈ 22 MiB for `double`).
2. **Alignment** – arrays should be aligned to at least a cache‑line (64 bytes) to avoid false sharing.
3. **Iterations** – the benchmark runs a configurable number of timed iterations (`NTIMES`).  The first iteration is discarded as a warm‑up; the remaining iterations are averaged.
4. **Timing** – wall‑clock timers are required; CPU timers are discouraged because of low resolution and systematic errors.
5. **Reporting** – results must be reported in the standard CSV‑friendly format:
   ```
   # Size: <MiB> MiB, Type: <float|double>, Iterations: <N>
   Kernel   Bytes/Iter  Time(s)   Bandwidth(GB/s)
   Copy     ...         ...       ...
   Scale    ...         ...       ...
   Add      ...         ...       ...
   Triad    ...         ...       ...
   ```
6. **Compliance** – results are considered *official* only if they follow these rules.  Modified or “tuned” versions must be clearly labelled.

## Problem Size Selection
The benchmark is intended to measure **main‑memory bandwidth**, so the arrays must be large enough that the data does not fit in any cache.  General rule of thumb:
- For a **single‑processor** system: `N ≥ max(4 * Σ(L2‑cache‑sizes), 1 000 000)` elements.
- For a **multi‑processor** system with `P` CPUs each having `C` bytes of last‑level cache: `N ≥ 4 * P * C` (rounded up to a convenient size, e.g. a power of two).
If the required size exceeds available RAM, the user should reduce the number of processors or use a smaller problem size and clearly note the deviation.

## Counting Bytes and FLOPs
STREAM counts **bytes requested by the program** (read + write), not the actual hardware traffic.  This makes results comparable across platforms.
- **Copy** – 2 × element size per iteration (no FLOPs).
- **Scale** – 2 × element size, 1 FLOP per element.
- **Add** – 3 × element size, 1 FLOP per element.
- **Triad** – 3 × element size, 2 FLOPs per element.

The bandwidth is calculated as:
```
bandwidth (GB/s) = (bytes_per_iteration) / (average_time) / 1e9
```
where `bytes_per_iteration` is taken from the table above.

## Compilation and Execution
```bash
# Compile the reference implementation (C version)
gcc -O3 -march=native -fopenmp stream.c -o stream
# Run with default parameters (2 000 000 elements, 10 iterations)
./stream
```
Options (relevant to the reference code):
- `-s <size>` – array size in elements (default 2 000 000).
- `-n <iters>` – number of timed iterations (default 10).
- `-t <type>` – `float` or `double`.
- `-O` – enable OpenMP for multi‑threaded runs.
- `-D TUNED` – build the “tuned” version that places each kernel in a separate routine.

## Parallel Execution
The reference code supports OpenMP; the number of threads is controlled via the environment variable `OMP_NUM_THREADS`.  For MPI runs a separate `stream_mpi` source is provided.  When using multiple threads or processes, the problem size must be scaled so that the total data accessed remains **≫** the combined cache size of all participants.

## References
- McCalpin, J. D. *Memory Bandwidth and Machine Balance in Current High Performance Computers*, IEEE Computer Society Technical Committee on Computer Architecture (TCCA) Newsletter, Dec 1995.
- McCalpin, J. D. *STREAM: Sustainable Memory Bandwidth in High Performance Computers*, technical report, University of Virginia, 1991‑2007.  <http://www.cs.virginia.edu/stream/>

---
*This specification is derived from the official STREAM documentation (http://www.cs.virginia.edu/stream/ref.html) and adapted for the memory‑vibes project.*