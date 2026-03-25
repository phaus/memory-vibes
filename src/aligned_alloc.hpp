// aligned_alloc.hpp
// Portable aligned memory allocation helpers.
// Provides aligned_alloc<T>(size, alignment) and aligned_free(void*).

#ifndef ALIGNED_ALLOC_HPP
#define ALIGNED_ALLOC_HPP

#include <cstddef>
#include <cstdlib>
#include <new>

namespace mem_band {

inline void aligned_free(void* ptr) noexcept {
#if defined(_WIN32) || defined(_WIN64)
    _aligned_free(ptr);
#else
    // For our fallback, the original pointer is stored just before the aligned pointer.
    if (ptr != nullptr) {
        void** real_ptr = static_cast<void**>(ptr);
        std::free(real_ptr[-1]);
    }
#endif
}

inline void* aligned_alloc(std::size_t size, std::size_t alignment) {
#if defined(_WIN32) || defined(_WIN64)
    // Windows: use _aligned_malloc; return nullptr on failure
    void* ptr = _aligned_malloc(size, alignment);
    if (!ptr) {
        return nullptr;
    }
    return ptr;
#else
    // Fallback: allocate extra space and align manually.
    // We assume that alignment is a power of two and >= sizeof(void*).
    // If not, we adjust.
    if (alignment < sizeof(void*)) {
        alignment = sizeof(void*);
    }
    // Make sure alignment is a power of two.
    if ((alignment & (alignment - 1)) != 0) {
        // Round up to next power of two.
        std::size_t new_align = 1;
        while (new_align < alignment) {
            new_align <<= 1;
        }
        alignment = new_align;
    }
    // We need to store the original pointer (to free later) so we add space for that.
    // We also need to align the returned pointer.
    // The offset is: alignment - 1 (to align) + sizeof(void*) (to store the original pointer).
    std::size_t offset = alignment - 1 + sizeof(void*);
    void* raw = std::malloc(size + offset);
    if (!raw) {
        return nullptr;
    }
    // Compute aligned pointer
    void** aligned_ptr = (void**)(((std::size_t)raw + offset) & ~(alignment - 1));
    // Store the original pointer just before the aligned pointer
    aligned_ptr[-1] = raw;
    return aligned_ptr;
#endif
}

} // namespace mem_band

#endif // ALIGNED_ALLOC_HPP