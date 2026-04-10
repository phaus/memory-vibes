#include "runtime_detection.hpp"
#include <dlfcn.h>
#include <cstring>
#include <cerrno>
#include <iomanip>
#include <sstream>

namespace mem_band {

void RuntimeDetector::register_feature(const Feature& feature) {
    features_.push_back(feature);
}

void RuntimeDetector::register_feature(std::string name, FeatureCheckFn checker, std::string description) {
    features_.push_back({std::move(name), std::move(checker), std::move(description)});
}

std::vector<RuntimeDetector::Feature> RuntimeDetector::get_available_features() const {
    std::vector<Feature> available;
    for (const auto& f : features_) {
        if (f.checker()) {
            available.push_back(f);
        }
    }
    return available;
}

std::vector<RuntimeDetector::Feature> RuntimeDetector::get_unavailable_features() const {
    std::vector<Feature> unavailable;
    for (const auto& f : features_) {
        if (!f.checker()) {
            unavailable.push_back(f);
        }
    }
    return unavailable;
}

std::vector<RuntimeDetector::Feature> RuntimeDetector::get_all_features() const {
    return features_;
}

bool RuntimeDetector::is_feature_available(const std::string& feature_name) const {
    for (const auto& f : features_) {
        if (f.name == feature_name) {
            return f.checker();
        }
    }
    return false;
}

DynamicLibraryLoader::LibraryHandle::~LibraryHandle() {
    close();
}

void DynamicLibraryLoader::LibraryHandle::close() {
    if (handle != nullptr) {
        dlclose(handle);
        handle = nullptr;
    }
}

DynamicLibraryLoader::~DynamicLibraryLoader() {
    for (auto& lib : loaded_libraries_) {
        lib.close();
    }
    loaded_libraries_.clear();
}

DynamicLibraryLoader::LibraryHandle DynamicLibraryLoader::open_library(
    const std::string& library_name, const std::string& search_path) {
    
    void* handle = nullptr;
    std::string full_path;

    if (!search_path.empty()) {
        full_path = search_path + "/" + library_name;
        handle = dlopen(full_path.c_str(), RTLD_LAZY | RTLD_LOCAL);
        if (handle == nullptr) {
            full_path.clear();
        }
    }

    if (handle == nullptr) {
        handle = dlopen(library_name.c_str(), RTLD_LAZY | RTLD_LOCAL);
    }

    if (handle != nullptr) {
        loaded_libraries_.emplace_back(handle, full_path);
        return LibraryHandle(handle, full_path);
    }

    return LibraryHandle();
}

std::vector<std::string> DynamicLibraryLoader::find_libraries(
    const std::string& library_name, const std::vector<std::string>& search_paths) {
    
    std::vector<std::string> found;
    
    for (const auto& path : search_paths) {
        std::string full_path = path + "/" + library_name;
        void* test = dlopen(full_path.c_str(), RTLD_LAZY | RTLD_LOCAL);
        if (test != nullptr) {
            dlclose(test);
            found.push_back(full_path);
        }
    }

    if (found.empty()) {
        void* test = dlopen(library_name.c_str(), RTLD_LAZY | RTLD_LOCAL);
        if (test != nullptr) {
            dlclose(test);
            found.push_back(library_name);
        }
    }

    return found;
}

std::string DynamicLibraryLoader::get_error() {
    const char* err = dlerror();
    return err ? std::string(err) : std::string("No error");
}

std::vector<std::string> DynamicLibraryLoader::get_default_search_paths() {
    std::vector<std::string> paths = {
        "/usr/lib",
        "/usr/lib64",
        "/usr/local/lib",
        "/usr/local/lib64",
        "/lib",
        "/lib64",
        "/opt/lib",
        "/opt/lib64"
    };

    char* ld_config = getenv("LD_LIBRARY_PATH");
    if (ld_config != nullptr) {
        std::string ld_path(ld_config);
        size_t pos = 0;
        while ((pos = ld_path.find(':')) != std::string::npos) {
            std::string segment = ld_path.substr(0, pos);
            if (!segment.empty()) {
                paths.push_back(segment);
            }
            ld_path.erase(0, pos + 1);
        }
        if (!ld_path.empty()) {
            paths.push_back(ld_path);
        }
    }

    return paths;
}

namespace runtime {

bool check_cuda_runtime() {
    DynamicLibraryLoader loader;
    auto paths = DynamicLibraryLoader::get_default_search_paths();
    paths.insert(paths.end(), {"/usr/local/cuda/lib64", "/usr/local/cuda/lib", "/opt/cuda/lib64"});
    
    auto libs = loader.find_libraries("libcudart.so", paths);
    return !libs.empty();
}

bool check_rocm_runtime() {
    DynamicLibraryLoader loader;
    auto paths = DynamicLibraryLoader::get_default_search_paths();
    paths.insert(paths.end(), {"/opt/rocm/lib", "/usr/lib/rocm", "/usr/local/lib/rocm"});
    
    auto libs = loader.find_libraries("libamdhip64.so", paths);
    return !libs.empty();
}

bool check_json_output() {
    DynamicLibraryLoader loader;
    auto paths = DynamicLibraryLoader::get_default_search_paths();
    auto libs = loader.find_libraries("libnlohmann_json.so", paths);
    return !libs.empty();
}

bool check_sqlite_output() {
    DynamicLibraryLoader loader;
    auto paths = DynamicLibraryLoader::get_default_search_paths();
    auto libs = loader.find_libraries("libsqlite3.so", paths);
    return !libs.empty();
}

std::vector<std::string> get_cuda_libraries() {
    std::vector<std::string> libs = {
        "libcudart.so",
        "libcublas.so",
        "libcusparse.so",
        "libcusolver.so"
    };
    return libs;
}

std::vector<std::string> get_rocm_libraries() {
    std::vector<std::string> libs = {
        "libamdhip64.so",
        "librocblas.so",
        "librocsolver.so"
    };
    return libs;
}

std::vector<RuntimeDetector::Feature> get_all_runtime_features() {
    RuntimeDetector detector;
    
    detector.register_feature(
        "CUDA Runtime",
        check_cuda_runtime,
        "NVIDIA CUDA runtime library available for GPU computing"
    );
    
    detector.register_feature(
        "ROCm Runtime",
        check_rocm_runtime,
        "AMD ROCm runtime library available for GPU computing"
    );
    
    detector.register_feature(
        "JSON Output",
        check_json_output,
        "nlohmann/json library available for JSON serialization"
    );
    
    detector.register_feature(
        "SQLite Output",
        check_sqlite_output,
        "SQLite3 library available for persistent storage"
    );
    
    return detector.get_all_features();
}

}

}
