#ifndef SRC_PLATFORM_DYNAMIC_LIB_HPP
#define SRC_PLATFORM_DYNAMIC_LIB_HPP

#include <string>
#include <vector>
#include <functional>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace mem_band {

class PlatformDynamicLib {
public:
    struct LibraryHandle {
#ifdef _WIN32
        HMODULE handle = nullptr;
#else
        void* handle = nullptr;
#endif
        std::string path;

        LibraryHandle() = default;
        explicit LibraryHandle(void* h, const std::string& p) : handle(reinterpret_cast<decltype(handle)>(h)), path(p) {}
        LibraryHandle(const LibraryHandle&) = delete;
        LibraryHandle& operator=(const LibraryHandle&) = delete;
        LibraryHandle(LibraryHandle&& other) noexcept : handle(other.handle), path(std::move(other.path)) {
            other.handle = nullptr;
        }
        LibraryHandle& operator=(LibraryHandle&& other) noexcept {
            if (this != &other) {
                close();
                handle = other.handle;
                path = std::move(other.path);
                other.handle = nullptr;
            }
            return *this;
        }
        ~LibraryHandle();

        void close();
        bool is_valid() const { return handle != nullptr; }

        template<typename T>
        T* get_symbol(const std::string& symbol_name) const;
    };

    PlatformDynamicLib() = default;
    ~PlatformDynamicLib();

    LibraryHandle open_library(const std::string& library_name, const std::string& search_path = "");
    std::vector<std::string> find_libraries(const std::string& library_name, const std::vector<std::string>& search_paths);

    static std::string get_error();
    static std::vector<std::string> get_default_search_paths();

private:
    std::vector<LibraryHandle> loaded_libraries_;
};

}

#endif
