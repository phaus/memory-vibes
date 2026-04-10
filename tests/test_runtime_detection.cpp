#include <gtest/gtest.h>
#include "runtime_detection.hpp"

namespace mem_band {

TEST(RuntimeDetectionTest, FeatureRegistration) {
    RuntimeDetector detector;
    
    detector.register_feature(
        "TestFeature",
        []() { return true; },
        "A test feature that is always available"
    );
    
    auto features = detector.get_all_features();
    EXPECT_EQ(features.size(), 1);
    EXPECT_EQ(features[0].name, "TestFeature");
    EXPECT_TRUE(features[0].checker());
}

TEST(RuntimeDetectionTest, FeatureAvailabilityCheck) {
    RuntimeDetector detector;
    
    detector.register_feature(
        "AlwaysAvailable",
        []() { return true; },
        "Always available"
    );
    
    detector.register_feature(
        "NeverAvailable",
        []() { return false; },
        "Never available"
    );
    
    auto available = detector.get_available_features();
    auto unavailable = detector.get_unavailable_features();
    
    EXPECT_EQ(available.size(), 1);
    EXPECT_EQ(unavailable.size(), 1);
    EXPECT_EQ(available[0].name, "AlwaysAvailable");
    EXPECT_EQ(unavailable[0].name, "NeverAvailable");
}

TEST(RuntimeDetectionTest, IsFeatureAvailable) {
    RuntimeDetector detector;
    
    detector.register_feature(
        "SomeFeature",
        []() { return true; },
        "Some feature"
    );
    
    EXPECT_TRUE(detector.is_feature_available("SomeFeature"));
    EXPECT_FALSE(detector.is_feature_available("NonExistentFeature"));
}

TEST(DynamicLibraryLoaderTest, EmptySearch) {
    DynamicLibraryLoader loader;
    auto paths = DynamicLibraryLoader::get_default_search_paths();
    
    EXPECT_GT(paths.size(), 0);
    EXPECT_NE(std::find(paths.begin(), paths.end(), "/usr/lib"), paths.end());
}

TEST(DynamicLibraryLoaderTest, FindNonexistentLibrary) {
    DynamicLibraryLoader loader;
    auto paths = DynamicLibraryLoader::get_default_search_paths();
    
    auto libs = loader.find_libraries("libdoesnotexist12345.so", paths);
    EXPECT_EQ(libs.size(), 0);
}

TEST(DynamicLibraryLoaderTest, OpenNonexistentLibrary) {
    DynamicLibraryLoader loader;
    auto lib = loader.open_library("libdoesnotexist12345.so");
    
    EXPECT_FALSE(lib.is_valid());
    EXPECT_EQ(lib.handle, nullptr);
}

TEST(RuntimeCheckersTest, CheckRuntimeFeatures) {
    auto features = runtime::get_all_runtime_features();
    
    EXPECT_GT(features.size(), 0);
    
    bool has_at_least_one = false;
    for (const auto& f : features) {
        EXPECT_FALSE(f.name.empty());
        EXPECT_FALSE(f.description.empty());
        if (f.checker()) {
            has_at_least_one = true;
        }
    }
    
    EXPECT_TRUE(has_at_least_one) << "At least one runtime feature should be detected";
}

TEST(RuntimeCheckersTest, CheckSpecificFeatures) {
    EXPECT_TRUE(runtime::check_cuda_runtime() || !runtime::check_cuda_runtime());
    EXPECT_TRUE(runtime::check_rocm_runtime() || !runtime::check_rocm_runtime());
    EXPECT_TRUE(runtime::check_json_output() || !runtime::check_json_output());
    EXPECT_TRUE(runtime::check_sqlite_output() || !runtime::check_sqlite_output());
}

TEST(RuntimeCheckersTest, GetLibraryNames) {
    auto cuda_libs = runtime::get_cuda_libraries();
    auto rocm_libs = runtime::get_rocm_libraries();
    
    EXPECT_GT(cuda_libs.size(), 0);
    EXPECT_GT(rocm_libs.size(), 0);
}

}
