#include "platform_detection.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <devguid.h>
#include <intrin.h>
#pragma intrinsic(__cpuid)
#else
#include <dirent.h>
#include <cstring>
#endif

#ifdef __APPLE__
#include <sys/sysctl.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
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
    int cpu_info[4] = {0};
    __cpuid(cpu_info, 0);

    if (cpu_info[1] == 0x756e6547 && cpu_info[3] == 0x49656e69 && cpu_info[2] == 0x6c65546e) {
        return "GenuineIntel";
    }

    __cpuid(cpu_info, 0x80000000);
    if (cpu_info[0] >= 0x80000004) {
        __cpuid(cpu_info, 0x80000002);
        std::string result(
            reinterpret_cast<char*>(cpu_info), 16
        );
        __cpuid(cpu_info, 0x80000003);
        result += std::string(
            reinterpret_cast<char*>(cpu_info), 16
        );
        __cpuid(cpu_info, 0x80000004);
        result += std::string(
            reinterpret_cast<char*>(cpu_info), 16
        );

        size_t start = result.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "Unknown";
        size_t end = result.find_last_not_of(" \t\r\n");
        std::string trimmed = result.substr(start, end - start + 1);
        if (trimmed.find("AMD") != std::string::npos) {
            return "AuthenticAMD";
        }
        if (trimmed.find("Intel") != std::string::npos) {
            return "GenuineIntel";
        }
    }

    __cpuid(cpu_info, 1);
    if ((cpu_info[3] & (1 << 31)) != 0) {
        return "AuthenticAMD";
    }

    return "Unknown";
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
    std::vector<HardwareDeviceInfo> devices;

    CFMutableDictionaryRef matching_dict = IOServiceMatching("IOPCIDevice");
    if (!matching_dict) {
        return devices;
    }

    io_iterator_t iter = MACH_PORT_NULL;
    kern_return_t kr = IOServiceGetMatchingServices(MACH_PORT_NULL, matching_dict, &iter);
    if (kr != KERN_SUCCESS || iter == MACH_PORT_NULL) {
        return devices;
    }

    io_object_t device_object;
    while ((device_object = IOIteratorNext(iter)) != MACH_PORT_NULL) {
        HardwareDeviceInfo info;

        CFTypeRef model_cf = IORegistryEntryCreateCFProperty(
            device_object, CFSTR("model"), kCFAllocatorDefault, 0
        );
        if (model_cf) {
            char model_buf[1024] = {0};
            if (CFGetTypeID(model_cf) == CFStringGetTypeID()) {
                CFStringGetCString(static_cast<CFStringRef>(model_cf), model_buf, sizeof(model_buf), kCFStringEncodingUTF8);
                info.device = std::string(model_buf);
            } else if (CFGetTypeID(model_cf) == CFDataGetTypeID()) {
                const UInt8* data = static_cast<const UInt8*>(CFDataGetBytePtr(static_cast<CFDataRef>(model_cf)));
                if (data) {
                    CFIndex data_len = CFDataGetLength(static_cast<CFDataRef>(model_cf));
                    char hex_buf[256] = {0};
                    int offset = 0;
                    for (CFIndex i = 0; i < data_len && offset < static_cast<CFIndex>(sizeof(hex_buf)) - 8; i++) {
                        offset += std::snprintf(hex_buf + offset, sizeof(hex_buf) - offset, "%02X", data[i]);
                    }
                    info.vendor_id = std::string(hex_buf);
                }
            }
            CFRelease(model_cf);
        }

        CFTypeRef name_cf = IORegistryEntryCreateCFProperty(
            device_object, CFSTR("name"), kCFAllocatorDefault, 0
        );
        if (name_cf) {
            if (CFGetTypeID(name_cf) == CFStringGetTypeID()) {
                char name_buf[256] = {0};
                CFStringGetCString(static_cast<CFStringRef>(name_cf), name_buf, sizeof(name_buf), kCFStringEncodingUTF8);
                info.bus_info = std::string(name_buf);
            }
            CFRelease(name_cf);
        }

        CFTypeRef compatible_cf = IORegistryEntryCreateCFProperty(
            device_object, CFSTR("compatible"), kCFAllocatorDefault, 0
        );
        if (compatible_cf) {
            if (CFGetTypeID(compatible_cf) == CFArrayGetTypeID()) {
                CFIndex count = CFArrayGetCount(static_cast<CFArrayRef>(compatible_cf));
                for (CFIndex i = 0; i < count; i++) {
                    CFStringRef item = static_cast<CFStringRef>(CFArrayGetValueAtIndex(static_cast<CFArrayRef>(compatible_cf), i));
                    if (CFGetTypeID(item) == CFStringGetTypeID()) {
                        char buf[512] = {0};
                        CFStringGetCString(item, buf, sizeof(buf), kCFStringEncodingUTF8);
                        std::string compatible_str(buf);
                        if (compatible_str.find("nvidia") != std::string::npos ||
                            compatible_str.find("NVIDIA") != std::string::npos) {
                            info.vendor = "NVIDIA";
                            info.vendor_id = "0x10de";
                        } else if (compatible_str.find("amd") != std::string::npos ||
                                   compatible_str.find("AMD") != std::string::npos) {
                            info.vendor = "AMD";
                            info.vendor_id = "0x1002";
                        } else if (compatible_str.find("intel") != std::string::npos ||
                                   compatible_str.find("Intel") != std::string::npos) {
                            info.vendor = "Intel";
                            info.vendor_id = "0x8086";
                        }
                        if (compatible_str.find("display") != std::string::npos ||
                            compatible_str.find("3d controller") != std::string::npos ||
                            compatible_str.find("VGA") != std::string::npos) {
                            info.class_info = "03";
                        }
                    }
                }
            }
            CFRelease(compatible_cf);
        }

        if (info.device.empty() && info.vendor.empty()) {
            io_registry_entry_t parent = MACH_PORT_NULL;
            kern_return_t kr = IORegistryEntryGetParentEntry(device_object, kIOServicePlane, &parent);
            if (kr == KERN_SUCCESS && parent != MACH_PORT_NULL) {
                CFTypeRef parent_model = IORegistryEntryCreateCFProperty(parent, CFSTR("model"), kCFAllocatorDefault, 0);
                if (parent_model && CFGetTypeID(parent_model) == CFStringGetTypeID()) {
                    char buf[1024] = {0};
                    CFStringGetCString(static_cast<CFStringRef>(parent_model), buf, sizeof(buf), kCFStringEncodingUTF8);
                    info.device = std::string(buf);
                }
                if (parent_model) CFRelease(parent_model);
                IOObjectRelease(parent);
            }
        }

        if (!info.vendor.empty() || !info.device.empty()) {
            devices.push_back(info);
        }

        IOObjectRelease(device_object);
    }

    IOObjectRelease(iter);
    return devices;
}
#endif

} // namespace mem_band
