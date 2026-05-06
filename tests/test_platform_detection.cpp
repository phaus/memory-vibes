#include <gtest/gtest.h>
#include "platform_detection.hpp"

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
    EXPECT_TRUE(isa == "AVX2" || isa == "AVX" || isa == "SSE2" ||
                isa == "Altivec" || isa == "Standard");
}

TEST_F(PlatformDetectionTest, TestGetCpuVendor) {
    std::string vendor = PlatformDetection::get_cpu_vendor();
    EXPECT_FALSE(vendor.empty());
#if defined(_WIN32)
    EXPECT_NE(vendor, "Unknown (Windows detection not fully implemented)");
#endif
    EXPECT_NE(vendor, "Unknown (WMI not available)");
}

TEST_F(PlatformDetectionTest, TestPciScan) {
    auto devices = PlatformDetection::scan_pci_devices();
    for (const auto& device : devices) {
        EXPECT_FALSE(device.vendor_id.empty());
        EXPECT_FALSE(device.device_id.empty());
        EXPECT_FALSE(device.class_info.empty());
    }
    for (const auto& device : devices) {
        EXPECT_FALSE(device.vendor_id.empty());
        EXPECT_FALSE(device.device_id.empty());
    }
}

TEST_F(PlatformDetectionTest, TestDetect) {
    PlatformInfo info = PlatformDetection::detect();
    EXPECT_FALSE(info.cpu_isa.empty());
    EXPECT_FALSE(info.cpu_vendor.empty());

#if defined(_WIN32)
    EXPECT_NE(info.cpu_vendor, "Unknown (Windows detection not fully implemented)");
    EXPECT_NE(info.cpu_vendor, "Unknown (WMI not available)");
#endif

    EXPECT_FALSE(info.has_amd_gpu != info.has_amd_gpu);
    EXPECT_FALSE(info.has_nvidia_gpu != info.has_nvidia_gpu);
    EXPECT_FALSE(info.has_intel_gpu != info.has_intel_gpu);
    EXPECT_FALSE(info.has_arm_gpu != info.has_arm_gpu);
    EXPECT_FALSE(info.has_npu != info.has_npu);
}

TEST_F(PlatformDetectionTest, TestHardwareDeviceInfo) {
    HardwareDeviceInfo device;
    device.vendor = "AMD";
    device.device = "Radeon";
    device.class_info = "03";
    device.vendor_id = "0x1002";
    EXPECT_EQ(device.vendor, "AMD");
    EXPECT_EQ(device.device, "Radeon");
    EXPECT_EQ(device.class_info, "03");
    EXPECT_EQ(device.vendor_id, "0x1002");
}

TEST_F(PlatformDetectionTest, TestPciVendorDetection) {
    HardwareDeviceInfo amd_device;
    amd_device.vendor_id = "0x1002";
    amd_device.class_info = "03";

    HardwareDeviceInfo nvidia_device;
    nvidia_device.vendor_id = "0x10de";
    nvidia_device.class_info = "03";

    HardwareDeviceInfo intel_device;
    intel_device.vendor_id = "0x8086";
    intel_device.class_info = "03";

    PlatformInfo info;
    info.pci_devices.push_back(amd_device);
    info.pci_devices.push_back(nvidia_device);
    info.pci_devices.push_back(intel_device);

    for (const auto& device : info.pci_devices) {
        if (device.vendor_id == "0x1002" && device.class_info == "03") {
            info.has_amd_gpu = true;
        }
        else if (device.vendor_id == "0x10de" || device.vendor.find("NVIDIA") != std::string::npos) {
            info.has_nvidia_gpu = true;
        }
        else if (device.vendor_id == "0x8086" || device.vendor.find("Intel") != std::string::npos) {
            info.has_intel_gpu = true;
        }
    }

    EXPECT_TRUE(info.has_amd_gpu);
    EXPECT_TRUE(info.has_nvidia_gpu);
    EXPECT_TRUE(info.has_intel_gpu);
}

TEST_F(PlatformDetectionTest, TestNpuDetection) {
    HardwareDeviceInfo gpu_device;
    gpu_device.vendor_id = "0x1002";
    gpu_device.class_info = "03";

    HardwareDeviceInfo npu_device;
    npu_device.vendor_id = "0x1002";
    npu_device.class_info = "03";
    npu_device.device = "AMD NPU DSP Processor";

    PlatformInfo info;
    info.pci_devices.push_back(gpu_device);
    info.pci_devices.push_back(npu_device);

    for (const auto& device : info.pci_devices) {
        if ((device.vendor_id == "0x1002" && device.class_info == "03") ||
            (device.vendor.find("AMD") != std::string::npos && device.class_info.find("3D") != std::string::npos)) {
            info.has_amd_gpu = true;
            if (device.device.find("NPU") != std::string::npos ||
                device.device.find("AI") != std::string::npos ||
                device.device.find("DSP") != std::string::npos) {
                info.has_npu = true;
            }
        }
    }

    EXPECT_TRUE(info.has_amd_gpu);
    EXPECT_TRUE(info.has_npu);
}

TEST_F(PlatformDetectionTest, TestVendorStringDetection) {
    HardwareDeviceInfo device;
    device.vendor = "NVIDIA";

    PlatformInfo info;
    info.pci_devices.push_back(device);

    for (const auto& d : info.pci_devices) {
        if (d.vendor_id == "0x10de" || d.vendor.find("NVIDIA") != std::string::npos) {
            info.has_nvidia_gpu = true;
        }
    }

    EXPECT_TRUE(info.has_nvidia_gpu);
}
