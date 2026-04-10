#include "runtime_detection.hpp"
#include "platform_dynamic_lib.hpp"
#include <iostream>

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

namespace runtime {

bool check_cuda_runtime();

bool check_cuda_runtime() {
    PlatformDynamicLib loader;
    auto paths = PlatformDynamicLib::get_default_search_paths();
    paths.insert(paths.end(), {"/usr/local/cuda/lib64", "/usr/local/cuda/lib", "/opt/cuda/lib64"});
#ifdef _WIN32
    paths.insert(paths.end(), {"C:\\Program Files\\NVIDIA Corporation", "C:\\Program Files (x86)\\NVIDIA Corporation"});
#endif
    
    auto libs = loader.find_libraries("cudart.dll", paths);
    if (libs.empty()) {
        libs = loader.find_libraries("libcudart.so", paths);
    }
#ifdef __APPLE__
    if (libs.empty()) {
        libs = loader.find_libraries("libcudart.dylib", paths);
    }
#endif
    return !libs.empty();
}

bool check_rocm_runtime();

bool check_rocm_runtime() {
    PlatformDynamicLib loader;
    auto paths = PlatformDynamicLib::get_default_search_paths();
    paths.insert(paths.end(), {"/opt/rocm/lib", "/usr/lib/rocm", "/usr/local/lib/rocm"});
#ifdef _WIN32
    paths.insert(paths.end(), {"C:\\Program Files\\ROCm"});
#endif
    
    auto libs = loader.find_libraries("hiprt64.dll", paths);
    if (libs.empty()) {
        libs = loader.find_libraries("libamdhip64.so", paths);
    }
#ifdef __APPLE__
    if (libs.empty()) {
        libs = loader.find_libraries("libamdhip64.dylib", paths);
    }
#endif
    return !libs.empty();
}

bool check_json_output();

bool check_json_output() {
    PlatformDynamicLib loader;
    auto paths = PlatformDynamicLib::get_default_search_paths();
    auto libs = loader.find_libraries("nlohmann_json.dll", paths);
    if (libs.empty()) {
        libs = loader.find_libraries("libnlohmann_json.so", paths);
    }
#ifdef __APPLE__
    if (libs.empty()) {
        libs = loader.find_libraries("libnlohmann_json.dylib", paths);
    }
#endif
    return !libs.empty();
}

bool check_sqlite_output();

bool check_sqlite_output() {
    PlatformDynamicLib loader;
    auto paths = PlatformDynamicLib::get_default_search_paths();
    auto libs = loader.find_libraries("sqlite3.dll", paths);
    if (libs.empty()) {
        libs = loader.find_libraries("libsqlite3.so", paths);
    }
#ifdef __APPLE__
    if (libs.empty()) {
        libs = loader.find_libraries("libsqlite3.dylib", paths);
    }
#endif
    return !libs.empty();
}

std::vector<std::string> get_cuda_libraries() {
    std::vector<std::string> libs = {
        "cudart.dll",
        "cublas.dll",
        "cusparse.dll",
        "cusolver.dll",
        "libcudart.so",
        "libcublas.so",
        "libcusparse.so",
        "libcusolver.so"
    };
    return libs;
}

std::vector<std::string> get_rocm_libraries() {
    std::vector<std::string> libs = {
        "hiprt64.dll",
        "rocblas.dll",
        "rocsolver.dll",
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
