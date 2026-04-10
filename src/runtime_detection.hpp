#ifndef SRC_RUNTIME_DETECTION_HPP
#define SRC_RUNTIME_DETECTION_HPP

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace mem_band {

class RuntimeDetector {
public:
    using FeatureCheckFn = std::function<bool()>;

    struct Feature {
        std::string name;
        FeatureCheckFn checker;
        std::string description;
    };

    RuntimeDetector() = default;

    void register_feature(const Feature& feature);
    void register_feature(std::string name, FeatureCheckFn checker, std::string description);

    std::vector<Feature> get_available_features() const;
    std::vector<Feature> get_unavailable_features() const;
    std::vector<Feature> get_all_features() const;

    bool is_feature_available(const std::string& feature_name) const;

private:
    std::vector<Feature> features_;
};

class DynamicLibraryLoader {
public:
    struct LibraryHandle {
        void* handle = nullptr;
        std::string path;

        LibraryHandle() = default;
        explicit LibraryHandle(void* h, const std::string& p) : handle(h), path(p) {}
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

    DynamicLibraryLoader() = default;
    ~DynamicLibraryLoader();

    LibraryHandle open_library(const std::string& library_name, const std::string& search_path = "");
    std::vector<std::string> find_libraries(const std::string& library_name, const std::vector<std::string>& search_paths);

    static std::string get_error();
    static std::vector<std::string> get_default_search_paths();

private:
    std::vector<LibraryHandle> loaded_libraries_;
};

namespace runtime {

bool check_cuda_runtime();
bool check_rocm_runtime();
bool check_json_output();
bool check_sqlite_output();

std::vector<std::string> get_cuda_libraries();
std::vector<std::string> get_rocm_libraries();

std::vector<RuntimeDetector::Feature> get_all_runtime_features();

}

}

#endif
