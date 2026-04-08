#include "system_info.hpp"
#include <iostream>
#include <sstream>
#include <vector>

SystemIdentifier SystemInfo::collect() {
    SystemIdentifier id;
    id.cpu_model = detect_cpu_model();
    id.core_count = detect_core_count();
    id.memory_size_mb = detect_memory_size();
    id.os_name = detect_os().first;
    id.os_version = detect_os().second;
    #if defined(_MSC_VER)
    id.compiler_version = "msvc " _MSC_VER;
#elif defined(__GNUC__)
    id.compiler_version = "gcc " __VERSION__;
#else
    id.compiler_version = "unknown compiler";
#endif
    id.platform = detect_platform();
    id.simd_enabled = true;
    id.benchmark_version = "1.0.0";
    return id;
}

void SystemInfo::print() {
    SystemIdentifier id = collect();
    std::cout << "# System Information\n";
    std::cout << "  CPU: " << id.cpu_model << "\n";
    std::cout << "  Cores: " << id.core_count << "\n";
    std::cout << "  Memory: " << id.memory_size_mb << " MB\n";
    std::cout << "  OS: " << id.os_name << "\n";
    if (!id.os_version.empty()) {
        std::cout << "  OS Version: " << id.os_version << "\n";
    }
    std::cout << "  Compiler: " << id.compiler_version << "\n";
    std::cout << "  Platform: " << id.platform << "\n";
    std::cout << "\nSystem ID: " << generate_hash(id) << "\n";
}

std::string SystemInfo::generate_hash(const SystemIdentifier& id) {
    std::ostringstream oss;
    oss << id.cpu_model << id.core_count << id.memory_size_mb 
        << id.os_name << id.compiler_version << id.platform;
    unsigned long hash = 5381;
    for (char c : oss.str()) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
    }
    std::ostringstream hash_oss;
    hash_oss << std::hex << (hash & 0xFFFFFFFF);
    return hash_oss.str();
}

std::string SystemInfo::detect_cpu_model() {
    std::cout << "COMPILE TEST\n";
#ifdef __aarch64__
    std::string result = "ARM 64-bit CPU";
    std::cout << "[DEBUG] : returning " << result << "\n";
    return result;
#elif defined(__x86_64__)
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string model = line.substr(pos + 1);
                model.erase(0, model.find_first_not_of(" \t"));
                model.erase(model.find_last_not_of(" \n\r\t") + 1);
                return model;
            }
        }
    }
    return "x86_64 CPU";
#else
    return "Unknown CPU";
#endif
}

int SystemInfo::detect_core_count() {
    return std::thread::hardware_concurrency();
}

long SystemInfo::detect_memory_size() {
#ifdef __linux__
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") != std::string::npos) {
            long mem_kb = 0;
            std::sscanf(line.c_str(), "MemTotal: %ld kB", &mem_kb);
            return mem_kb / 1024;
        }
    }
#endif
#ifdef __APPLE__
    size_t size = 0;
    sysctlbyname("hw.memsize", &size, nullptr, nullptr, 0);
    return size / (1024 * 1024);
#endif
    return 1;  // Fallback
}

std::pair<std::string, std::string> SystemInfo::detect_os() {
#ifdef __linux__
    std::ifstream os_release("/etc/os-release");
    std::string name, version;
    std::string line;
    while (std::getline(os_release, line)) {
        if (line.find("NAME=") != std::string::npos) {
            name = line.substr(5);
            name.erase(0, name.find_first_not_of('"'));
            name.erase(name.find_last_not_of('"') + 1);
        }
        if (line.find("VERSION=") != std::string::npos) {
            version = line.substr(8);
            version.erase(0, version.find_first_not_of('"'));
            version.erase(version.find_last_not_of('"') + 1);
        }
    }
    return {name.empty() ? "Linux" : name, version};
#endif
#ifdef __APPLE__
    return {"macOS", "Unknown"};
#endif
    return {"Unknown", "Unknown"};
}

std::string SystemInfo::detect_platform() {
#ifdef __aarch64__
    return "aarch64-linux";
#elif defined(__x86_64__)
    return "x86_64-linux";
#elif defined(_WIN32)
    return "x86_64-windows";
#else
    return "unknown-unknown";
#endif
}
