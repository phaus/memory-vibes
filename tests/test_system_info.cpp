#include <gtest/gtest.h>
#include "system_info.hpp"

using namespace mem_band;

namespace {

class SystemInfoTest : public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(SystemInfoTest, CPUModelDetection) {
    auto sysinfo = SystemInfo::collect();
    EXPECT_FALSE(sysinfo.cpu_model.empty());
    EXPECT_FALSE(sysinfo.cpu_model.find("Unknown") == std::string::npos);
}

TEST_F(SystemInfoTest, CoreCountPositive) {
    auto sysinfo = SystemInfo::collect();
    EXPECT_GT(sysinfo.core_count, 0);
}

TEST_F(SystemInfoTest, MemorySizePositive) {
    auto sysinfo = SystemInfo::collect();
    EXPECT_GT(sysinfo.memory_size_mb, 0);
}

TEST_F(SystemInfoTest, OSNameNotEmpty) {
    auto sysinfo = SystemInfo::collect();
    EXPECT_FALSE(sysinfo.os_name.empty());
}

TEST_F(SystemInfoTest, CompilerVersionNotEmpty) {
    auto sysinfo = SystemInfo::collect();
    EXPECT_FALSE(sysinfo.compiler_version.empty());
}

TEST_F(SystemInfoTest, PlatformNotEmpty) {
    auto sysinfo = SystemInfo::collect();
    EXPECT_FALSE(sysinfo.platform.empty());
}

TEST_F(SystemInfoTest, SystemIDHashGeneration) {
    auto sysinfo = SystemInfo::collect();
    auto hash = SystemInfo::generate_hash(sysinfo);
    EXPECT_EQ(hash.length(), 8);  // 4-byte hash in hex = 8 chars
    for (char c : hash) {
        EXPECT_TRUE(std::isxdigit(static_cast<unsigned char>(c)) != 0);
    }
}

TEST_F(SystemInfoTest, SystemIDConsistency) {
    auto sysinfo = SystemInfo::collect();
    auto hash1 = SystemInfo::generate_hash(sysinfo);
    auto hash2 = SystemInfo::generate_hash(sysinfo);
    EXPECT_EQ(hash1, hash2);  // Same system should produce same hash
}

} // namespace
