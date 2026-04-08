#ifndef PLATFORM_DETECTION_HPP
#define PLATFORM_DETECTION_HPP

#include <string>
#include <vector>
#include <utility>

namespace mem_band {

struct HardwareDeviceInfo {
    std::string vendor;
    std::string device;
    std::string class_info;
    std::string bus_info;
    std::string vendor_id;
    std::string device_id;
};

struct PlatformInfo {
    std::string cpu_isa;
    std::string cpu_vendor;
    std::vector<HardwareDeviceInfo> pci_devices;
    bool has_amd_gpu;
    bool has_nvidia_gpu;
    bool has_intel_gpu;
    bool has_arm_gpu;
    bool has_npu;
};

class PlatformDetection {
public:
    static PlatformInfo detect();
    static std::string get_cpu_isa();
    static std::string get_cpu_vendor();
    static std::vector<HardwareDeviceInfo> scan_pci_devices();

private:
#if defined(__linux__)
    static std::string detect_cpu_vendor_linux();
    static std::vector<HardwareDeviceInfo> scan_pci_linux();
    static bool parse_sys_device_info(const std::string& path, HardwareDeviceInfo& device);
#endif

#if defined(_WIN32)
    static std::string detect_cpu_vendor_windows();
    static std::vector<HardwareDeviceInfo> scan_pci_windows();
#endif

#if defined(__APPLE__)
    static std::string detect_cpu_vendor_macos();
    static std::vector<HardwareDeviceInfo> scan_pci_macos();
#endif

    static HardwareDeviceInfo parse_device_info(const std::string& line);
};

} // namespace mem_band

#endif // PLATFORM_DETECTION_HPP
