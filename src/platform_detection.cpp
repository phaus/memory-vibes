#include "platform_detection.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <devguid.h>
#include <wbemidl.h>
#include <intrin.h>
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
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
namespace {

std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), nullptr, 0);
    if (len <= 0) return {};
    std::wstring wide(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), &wide[0], len);
    return wide;
}

std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string utf8(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), &utf8[0], len, nullptr, nullptr);
    return utf8;
}

std::string BstrToStdString(BSTR bstr) {
    if (!bstr) return std::string();
    std::string result = WideToUtf8(bstr);
    SysFreeString(bstr);
    return result;
}

std::string VariantToStdString(const VARIANT& variant) {
    std::string result;
    switch (variant.vt) {
        case VT_BSTR:
            result = BstrToStdString(variant.bstrVal);
            break;
        case VT_STRING:
            result = BstrToStdString(variant.bstrVal);
            break;
        case VT_UI4:
            result = std::to_string(variant.ulVal);
            break;
        case VT_UI2:
            result = std::to_string(variant.usVal);
            break;
        case VT_I4:
            result = std::to_string(variant.lVal);
            break;
        case VT_EMPTY:
        case VT_NULL:
            result = "";
            break;
        default:
            result = "";
            break;
    }
    return result;
}

std::string TrimW(const std::wstring& s) {
    size_t start = s.find_first_not_of(L" \t\r\n");
    if (start == std::wstring::npos) return "";
    size_t end = s.find_last_not_of(L" \t\r\n");
    return WideToUtf8(s.substr(start, end - start + 1));
}

std::string Trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::vector<HardwareDeviceInfo> ScanPciDevicesWmi() {
    std::vector<HardwareDeviceInfo> devices;
    
    HRESULT hres = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hres)) return devices;

    hres = CoInitializeSecurity(NULL, -1, NULL, NULL,
                                RPC_C_AUTHN_LEVEL_DEFAULT,
                                RPC_C_IMP_LEVEL_IMPERSONATE,
                                NULL, EOAC_NONE, NULL);

    if (FAILED(hres)) {
        CoUninitialize();
        return devices;
    }

    IWbemLocator* pLocator = nullptr;
    hres = CoCreateInstance(
        __uuidOf(WbemLocator),
        NULL, CLSCTX_INPROC_SERVER,
        __uuidOf(IWbemLocator),
        reinterpret_cast<void**>(&pLocator)
    );

    if (FAILED(hres)) {
        CoUninitialize();
        return devices;
    }

    IWbemServices* pServices = nullptr;
    hres = pLocator->ConnectServer(
        bstr_t(L"ROOT\\CIMV2"),
        NULL, NULL, 0, NULL,
        wbemConnectFlag, NULL, &pServices
    );

    pLocator->Release();

    if (FAILED(hres)) {
        CoUninitialize();
        return devices;
    }

    hres = CoSetProxyBlanket(
        pServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
        NULL, RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE
    );

    if (FAILED(hres)) {
        pServices->Release();
        CoUninitialize();
        return devices;
    }

    IEnumWbemClassObject* pEnumerator = nullptr;
    hres = pServices->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_PNPEntity WHERE ClassGuid = '{88BAE032-5A81-49F0-BC3D-A6FF1A47C4E0}' OR ClassGuid = '{4D36E968-E325-11CE-BFC1-08002BE10318}'"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL, &pEnumerator
    );

    if (FAILED(hres)) {
        pServices->Release();
        CoUninitialize();
        return devices;
    }

    IWbemClassObject* pclsObj = nullptr;
    ULONG uReturn = 0;

    while (pEnumerator) {
        hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn) break;

        HardwareDeviceInfo device;
        VARIANT var;
        VariantInit(&var);

        if (SUCCEEDED(pclsObj->Get(L"Name", 0, &var, NULL, NULL))) {
            device.device = Trim(VariantToStdString(var));
            VariantClear(&var);
        }

        if (SUCCEEDED(pclsObj->Get(L"Vendor", 0, &var, NULL, NULL))) {
            std::string vendor = Trim(VariantToStdString(var));
            if (vendor == "Microsoft") {
                if (SUCCEEDED(pclsObj->Get(L"Manufacturer", 0, &var, NULL, NULL))) {
                    vendor = Trim(VariantToStdString(var));
                    VariantClear(&var);
                }
            }
            device.vendor = vendor;
            VariantClear(&var);
        }

        if (SUCCEEDED(pclsObj->Get(L"PNPDeviceID", 0, &var, NULL, NULL))) {
            std::string pnp_id = Trim(VariantToStdString(var));
            device.bus_info = pnp_id;
            if (pnp_id.find("VEN_") != std::string::npos && pnp_id.find("DEV_") != std::string::npos) {
                size_t ven_pos = pnp_id.find("VEN_");
                if (ven_pos != std::string::npos) {
                    std::string ven_part = pnp_id.substr(ven_pos + 4, 4);
                    if (ven_part.length() == 4) {
                        char hex[6] = {0};
                        std::snprintf(hex, sizeof(hex), "0x%s", ven_part.c_str());
                        device.vendor_id = std::string(hex);
                    }
                }
                size_t dev_pos = pnp_id.find("DEV_");
                if (dev_pos != std::string::npos) {
                    std::string dev_part = pnp_id.substr(dev_pos + 4, 4);
                    if (dev_part.length() == 4) {
                        char hex[6] = {0};
                        std::snprintf(hex, sizeof(hex), "0x%s", dev_part.c_str());
                        device.device_id = std::string(hex);
                    }
                }
            }
            VariantClear(&var);
        }

        if (SUCCEEDED(pclsObj->Get(L"Class", 0, &var, NULL, NULL))) {
            device.class_info = Trim(VariantToStdString(var));
            VariantClear(&var);
        }

        if (!device.vendor.empty() || !device.device.empty()) {
            devices.push_back(device);
        }

        pclsObj->Release();
    }

    pEnumerator->Release();
    pServices->Release();
    CoUninitialize();

    if (devices.empty()) {
        return ScanPciDevicesSetupApi();
    }

    return devices;
}

std::vector<HardwareDeviceInfo> ScanPciDevicesSetupApi() {
    std::vector<HardwareDeviceInfo> devices;

    HDEVINFO dev_info = SetupDiGetClassDevs(
        NULL, "PCI", NULL,
        DIGCF_PRESENT | DIGCF_ALLCLASSES
    );

    if (dev_info == INVALID_HANDLE_VALUE) {
        return devices;
    }

    SP_DEVINFO_DATA dev_data;
    dev_data.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInfo(dev_info, i, &dev_data); i++) {
        HardwareDeviceInfo device;

        TCHAR buffer[MAX_PATH] = {0};
        if (SetupDiGetDeviceRegistryProperty(
                dev_info, &dev_data, SPDRP_HARDWAREID,
                NULL, (PBYTE)buffer, sizeof(buffer), NULL)) {
            std::string hw_id = WideToUtf8(buffer);
            device.bus_info = hw_id;

            size_t ven_pos = hw_id.find("VEN_");
            if (ven_pos != std::string::npos) {
                std::string ven_part = hw_id.substr(ven_pos + 4, 4);
                if (ven_part.length() == 4) {
                    char hex[6] = {0};
                    std::snprintf(hex, sizeof(hex), "0x%s", ven_part.c_str());
                    device.vendor_id = std::string(hex);
                }
            }
            size_t dev_pos = hw_id.find("DEV_");
            if (dev_pos != std::string::npos) {
                std::string dev_part = hw_id.substr(dev_pos + 4, 4);
                if (dev_part.length() == 4) {
                    char hex[6] = {0};
                    std::snprintf(hex, sizeof(hex), "0x%s", dev_part.c_str());
                    device.device_id = std::string(hex);
                }
            }
        }

        if (SetupDiGetDeviceRegistryProperty(
                dev_info, &dev_data, SPDRP_DEVICEDESC,
                NULL, (PBYTE)buffer, sizeof(buffer), NULL)) {
            device.device = Trim(WideToUtf8(buffer));
        }

        if (SetupDiGetDeviceRegistryProperty(
                dev_info, &dev_data, SPDRP_MFG,
                NULL, (PBYTE)buffer, sizeof(buffer), NULL)) {
            device.vendor = Trim(WideToUtf8(buffer));
        }

        const ULONG class_codes[] = {0x030000, 0x038000, 0x030100, 0x030200};
        for (ULONG cc : class_codes) {
            TCHAR class_buf[MAX_PATH] = {0};
            if (SetupDiGetDeviceRegistryProperty(
                    dev_info, &dev_data, SPDRP_CLASS,
                    NULL, (PBYTE)class_buf, sizeof(class_buf), NULL) &&
                SetupDiGetDeviceRegistryProperty(
                    dev_info, &dev_data, SPDRP_CLASSCODE,
                    NULL, (PBYTE)class_buf, sizeof(class_buf), NULL)) {
                break;
            }
        }

        if (!device.vendor.empty() || !device.device.empty()) {
            devices.push_back(device);
        }
    }

    SetupDiDestroyDeviceInfoList(dev_info);
    return devices;
}

} // anonymous namespace

std::string PlatformDetection::detect_cpu_vendor_windows() {
    char vendor_buffer[49] = {0};
    unsigned int cpu_info[4] = {0};
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

        std::string trimmed = Trim(result);
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

    return "Unknown (WMI not available)";
}

std::vector<HardwareDeviceInfo> PlatformDetection::scan_pci_windows() {
    return ScanPciDevicesWmi();
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
            io_object_t parent = IORegistryEntryGetParentEntry(device_object, kIOServicePlane);
            if (parent) {
                CFTypeRef parent_model = IORegistryEntryCreateCFProperty(parent, CFSTR("model"), kCFAllocatorDefault, 0);
                if (parent_model && CFGetTypeID(parent_model) == CFStringGetTypeID()) {
                    char buf[1024] = {0};
                    CFStringGetCString(static_cast<CFStringRef>(parent_model), buf, sizeof(buf), kCFStringEncodingUTF8);
                    info.device = std::string(buf);
                }
                if (parent_model) CFRelease(parent_model);
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
