#include "platform_detection.hpp"
#include <gtest/gtest.h>

using namespace mem_band;

class PlatformDetectionTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(PlatformDetectionTest, TestGetCpuIsa) {
    std::string isa = PlatformDetection::get_cpu_isa();
    EXPECT_FALSE(isa.empty());
    EXPECT_NE(isa, "Unknown");
}

TEST_F(PlatformDetectionTest, TestGetCpuVendor) {
    std::string vendor = PlatformDetection::get_cpu_vendor();
    EXPECT_FALSE(vendor.empty());
}

TEST_F(PlatformDetectionTest, TestPciScan) {
    auto devices = PlatformDetection::scan_pci_devices();
    EXPECT_TRUE(devices.size() >= 0);
}

TEST_F(PlatformDetectionTest, TestDetect) {
    PlatformInfo info = PlatformDetection::detect();
    EXPECT_FALSE(info.cpu_isa.empty());
    EXPECT_FALSE(info.cpu_vendor.empty());
    
    EXPECT_EQ(info.has_amd_gpu, info.has_amd_gpu);
    EXPECT_EQ(info.has_nvidia_gpu, info.has_nvidia_gpu);
    EXPECT_EQ(info.has_intel_gpu, info.has_intel_gpu);
    EXPECT_EQ(info.has_arm_gpu, info.has_arm_gpu);
}

TEST_F(PlatformDetectionTest, TestHardwareDeviceInfo) {
    HardwareDeviceInfo device;
    device.vendor = "AMD";
    device.device = "Radeon";
    EXPECT_EQ(device.vendor, "AMD");
    EXPECT_EQ(device.device, "Radeon");
}
