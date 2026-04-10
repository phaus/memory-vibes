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
