// aligned_alloc.hpp
// Portable aligned memory allocation helpers.
// Provides aligned_alloc<T>(size, alignment) and aligned_free(void*).

#ifndef ALIGNED_ALLOC_HPP
#define ALIGNED_ALLOC_HPP

#include <cstddef>
#include <cstdlib>
#include <new>
#include <stdexcept>

namespace mem_band {

inline void aligned_free(void* ptr) noexcept {
#if defined(_WIN32) || defined(_WIN64)
    _aligned_free(ptr);
#else
    std::free(ptr);
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
    // POSIX: use posix_memalign; return nullptr on failure
    void* ptr = nullptr;
    int err = posix_memalign(&ptr, alignment, size);
    if (err != 0) {
        return nullptr;
    }
    return ptr;
#endif
}

} // namespace mem_band

#endif // ALIGNED_ALLOC_HPP