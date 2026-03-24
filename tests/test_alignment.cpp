#include <cassert>
#include <cstddef>
#include <cstdint>
#include "aligned_alloc.hpp"

int main() {
    const std::size_t size = 1024; // 1 KiB
    const std::size_t alignment = 64; // cache line
    void* ptr = mem_band::aligned_alloc(size, alignment);
    assert(ptr && "Allocation failed");
    // Verify alignment
    std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(ptr);
    assert((addr % alignment) == 0 && "Pointer not aligned to cache line");
    mem_band::aligned_free(ptr);
    return 0;
}
