#include "platform_detection.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <devguid.h>
#else
#include <dirent.h>
#include <cstring>
#endif

#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

namespace mem_band {

std::string PlatformDetection::get_cpu_isa() {
#if defined(__AVX2__)
    return "AVX2";
#elif defined(__AVX__)
    return "AVX";
#elif defined(__SSE2__)
    return "SSE2";
#elif defined(__ALTIVEC__)
    return "Altivec";
#else
    return "Standard";
#endif
}

std::string PlatformDetection::get_cpu_vendor() {
#if defined(__linux__)
    return detect_cpu_vendor_linux();
#elif defined(_WIN32)
    return detect_cpu_vendor_windows();
#elif defined(__APPLE__)
    return detect_cpu_vendor_macos();
#else
    return "Unknown";
#endif
}

PlatformInfo PlatformDetection::detect() {
    PlatformInfo info;
    info.cpu_isa = get_cpu_isa();
    info.cpu_vendor = get_cpu_vendor();
    info.pci_devices = scan_pci_devices();
    
    info.has_amd_gpu = false;
    info.has_nvidia_gpu = false;
    info.has_intel_gpu = false;
    info.has_arm_gpu = false;
    info.has_npu = false;
    
    for (const auto& device : info.pci_devices) {
        if ((device.vendor_id == "0x1002" && device.class_info == "03") ||
            (device.vendor.find("AMD") != std::string::npos && device.class_info.find("3D") != std::string::npos)) {
            info.has_amd_gpu = true;
            if (device.class_info.find("NPU") != std::string::npos ||
                device.class_info.find("AI") != std::string::npos ||
                device.class_info.find("DSP") != std::string::npos) {
                info.has_npu = true;
            }
        }
        else if (device.vendor_id == "0x10de" || device.vendor.find("NVIDIA") != std::string::npos) {
            info.has_nvidia_gpu = true;
        }
        else if (device.vendor_id == "0x8086" || device.vendor.find("Intel") != std::string::npos) {
            info.has_intel_gpu = true;
        }
        else if (device.vendor_id == "0x13b5" || device.vendor.find("ARM") != std::string::npos) {
            info.has_arm_gpu = true;
        }
    }
    
    return info;
}

std::vector<HardwareDeviceInfo> PlatformDetection::scan_pci_devices() {
#if defined(__linux__)
    return scan_pci_linux();
#elif defined(_WIN32)
    return scan_pci_windows();
#elif defined(__APPLE__)
    return scan_pci_macos();
#else
    return std::vector<HardwareDeviceInfo>();
#endif
}

#if defined(__linux__)

std::string PlatformDetection::detect_cpu_vendor_linux() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("vendor_id") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string vendor = line.substr(pos + 1);
                vendor.erase(0, vendor.find_first_not_of(" \t"));
                vendor.erase(vendor.find_last_not_of(" \n\r\t") + 1);
                return vendor.empty() ? "Unknown" : vendor;
            }
        }
    }
    return "Unknown";
}

std::vector<HardwareDeviceInfo> PlatformDetection::scan_pci_linux() {
    std::vector<HardwareDeviceInfo> devices;
    const std::string sys_path = "/sys/bus/pci/devices";
    
    DIR* dir = opendir(sys_path.c_str());
    if (!dir) {
        return devices;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;
        
        std::string device_path = sys_path + "/" + name;
        HardwareDeviceInfo device;
        if (parse_sys_device_info(device_path, device)) {
            devices.push_back(device);
        }
    }
    
    closedir(dir);
    return devices;
}

bool PlatformDetection::parse_sys_device_info(const std::string& path, HardwareDeviceInfo& device) {
    std::string vendor_path = path + "/vendor";
    std::string device_path = path + "/device";
    std::string class_path = path + "/class";
    std::string bus_path = path + "/bus_info";
    
    auto read_file = [](const std::string& file) -> std::string {
        std::ifstream f(file);
        std::string content;
        if (f.is_open()) {
            std::getline(f, content);
            content.erase(0, content.find_first_not_of(" \t"));
            content.erase(content.find_last_not_of(" \n\r\t") + 1);
        }
        return content;
    };
    
    device.vendor = read_file(vendor_path);
    device.device = read_file(device_path);
    device.class_info = read_file(class_path);
    device.bus_info = read_file(bus_path);
    
    if (!device.vendor.empty() && std::strncmp(device.vendor.c_str(), "0x", 2) == 0) {
        device.vendor_id = device.vendor;
    }
    
    if (!device.device.empty() && std::strncmp(device.device.c_str(), "0x", 2) == 0) {
        device.device_id = device.device;
    }
    
    return !device.vendor.empty() || !device.device.empty();
}

#endif

#if defined(_WIN32)
std::string PlatformDetection::detect_cpu_vendor_windows() {
    return "Unknown (Windows detection not fully implemented)";
}

std::vector<HardwareDeviceInfo> PlatformDetection::scan_pci_windows() {
    return std::vector<HardwareDeviceInfo>();
}
#endif

#if defined(__APPLE__)
std::string PlatformDetection::detect_cpu_vendor_macos() {
    char hw_vendor[256] = {0};
    size_t len = sizeof(hw_vendor);
    if (sysctlbyname("machdep.cpu.vendor", hw_vendor, &len, nullptr, 0) == 0) {
        return std::string(hw_vendor);
    }
    return "Apple";
}

std::vector<HardwareDeviceInfo> PlatformDetection::scan_pci_macos() {
    return std::vector<HardwareDeviceInfo>();
}
#endif

HardwareDeviceInfo PlatformDetection::parse_device_info(const std::string& line) {
    HardwareDeviceInfo device;
    return device;
}

} // namespace mem_band
