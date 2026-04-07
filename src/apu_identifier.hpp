// apu_identifier.hpp
// APU system identifier collection for AMD Strix Point / Halo platforms.
// Collects CPU model, memory size, and APU platform information.

#ifndef APU_IDENTIFIER_HPP
#define APU_IDENTIFIER_HPP

#include <string>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#if defined(__APPLE__)
#include <sys/sysctl.h>
#endif

namespace mem_band {

/**
 * @brief Structure containing APU system identification information
 */
struct APUSystemInfo {
    std::string cpu_model;           // Full CPU model string
    std::string apu_platform;        // Detected APU platform (Strix Point, Halo, etc.)
    std::size_t total_memory_bytes;  // Total system memory in bytes
    std::string linux_distro;        // Linux distribution info (if applicable)
    
    bool is_strix_point() const {
        return apu_platform.find("Strix") != std::string::npos;
    }
    
    bool is_halo() const {
        return apu_platform.find("Halo") != std::string::npos;
    }
};

/**
 * @brief Read all lines from a file
 * @param filepath Path to the file
 * @return Vector of lines
 */
static std::vector<std::string> read_file_lines(const std::string& filepath) {
    std::vector<std::string> lines;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return lines;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

/**
 * @brief Get CPU model from /proc/cpuinfo (Linux) or lscpu
 * @return CPU model string
 */
static std::string get_cpu_model() {
    std::string cpu_model = "<unknown>";
    
#if defined(__linux__)
    // Try reading /proc/cpuinfo first
    auto lines = read_file_lines("/proc/cpuinfo");
    for (const auto& line : lines) {
        if (line.find("model name") == 0) {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                cpu_model = line.substr(colon_pos + 1);
                // Trim leading whitespace
                size_t start = cpu_model.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    cpu_model = cpu_model.substr(start);
                }
            }
            break;
        }
    }
    
    // Fallback to lscpu if /proc/cpuinfo didn't work
    if (cpu_model == "<unknown>") {
        std::ifstream proc("/proc/self/environ");
        // Use lscpu as fallback
        char buffer[256];
        FILE* pipe = popen("lscpu | grep 'Model name:' 2>/dev/null", "r");
        if (pipe) {
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                std::string line(buffer);
                size_t colon_pos = line.find(':');
                if (colon_pos != std::string::npos) {
                    cpu_model = line.substr(colon_pos + 1);
                    size_t start = cpu_model.find_first_not_of(" \t");
                    if (start != std::string::npos) {
                        cpu_model = cpu_model.substr(start);
                    }
                }
            }
            pclose(pipe);
        }
    }
    
#elif defined(_WIN32)
    // Windows: Use WMI or registry (not implemented - returns unknown)
    cpu_model = "<unknown>";
    
#elif defined(__APPLE__)
    // macOS: Use sysctl
    char buffer[1024];
    size_t len = sizeof(buffer);
    if (sysctlbyname("machdep.cpu.brand_string", buffer, &len, nullptr, 0) == 0) {
        cpu_model = buffer;
    }
#endif
    
    return cpu_model;
}

/**
 * @brief Get total system memory in bytes
 * @return Memory size in bytes
 */
static std::size_t get_total_memory() {
    std::size_t total_bytes = 0;
    
#if defined(__linux__)
    // Read /proc/meminfo
    auto lines = read_file_lines("/proc/meminfo");
    for (const auto& line : lines) {
        if (line.find("MemTotal:") == 0) {
            std::istringstream iss(line);
            std::string key;
            std::size_t value_kb;
            if (iss >> key >> value_kb) {
                total_bytes = static_cast<std::size_t>(value_kb) * 1024;
            }
            break;
        }
    }
    
    // Fallback to getpagesize * number of pages
    if (total_bytes == 0) {
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGESIZE);
        if (pages > 0 && page_size > 0) {
            total_bytes = static_cast<std::size_t>(pages) * static_cast<std::size_t>(page_size);
        }
    }
    
#elif defined(_WIN32)
    // Windows: GlobalMemoryStatusEx (not implemented)
    total_bytes = 0;
    
#elif defined(__APPLE__)
    // macOS: Use sysctl
    long pages = 0;
    size_t len = sizeof(pages);
    if (sysctlbyname("hw.physmem", &pages, &len, nullptr, 0) == 0) {
        total_bytes = static_cast<std::size_t>(pages);
    }
#endif
    
    return total_bytes;
}

/**
 * @brief Detect APU platform from CPU model
 * @param cpu_model CPU model string
 * @return Detected APU platform name
 */
static std::string detect_apu_platform(const std::string& cpu_model) {
    std::string lower_model = cpu_model;
    std::transform(lower_model.begin(), lower_model.end(), lower_model.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    if (lower_model.find("strix") != std::string::npos) {
        return "AMD Ryzen AI Strix Point";
    }
    if (lower_model.find("halo") != std::string::npos) {
        return "AMD Ryzen AI Halo";
    }
    if (lower_model.find("ryzen ai") != std::string::npos) {
        return "AMD Ryzen AI";
    }
    if (lower_model.find("amd") != std::string::npos) {
        return "AMD APU";
    }
    
    return "<unknown>";
}

/**
 * @brief Get Linux distribution name
 * @return Distribution string or empty
 */
static std::string get_linux_distribution() {
#if defined(__linux__)
    auto lines = read_file_lines("/etc/os-release");
    for (const auto& line : lines) {
        if (line.find("PRETTY_NAME") == 0) {
            size_t quote_start = line.find('"');
            size_t quote_end = line.find('"', quote_start + 1);
            if (quote_start != std::string::npos && quote_end != std::string::npos) {
                return line.substr(quote_start + 1, quote_end - quote_start - 1);
            }
        }
    }
    
    // Try other common files
    auto distros = read_file_lines("/etc/lsb-release");
    for (const auto& line : distros) {
        if (line.find("DISTRIB_DESCRIPTION") == 0) {
            size_t quote_start = line.find('"');
            if (quote_start != std::string::npos) {
                size_t quote_end = line.find('"', quote_start + 1);
                if (quote_end != std::string::npos) {
                    return line.substr(quote_start + 1, quote_end - quote_start - 1);
                }
            }
        }
    }
#endif
    return "";
}

/**
 * @brief Collect all APU system identification information
 * @return APUSystemInfo structure with collected data
 */
inline APUSystemInfo collect_apu_system_info() {
    APUSystemInfo info;
    
    info.cpu_model = get_cpu_model();
    info.apu_platform = detect_apu_platform(info.cpu_model);
    info.total_memory_bytes = get_total_memory();
    info.linux_distro = get_linux_distribution();
    
    return info;
}

/**
 * @brief Print system information in a benchmark-friendly format
 * @param info System info to print
 * @param os Output stream (default: stdout)
 */
inline void print_system_info(const APUSystemInfo& info, std::ostream& os = ::std::cout) {
    os << "# System Information\n";
    os << "  CPU: " << info.cpu_model << ::std::endl;
    os << "  Platform: " << info.apu_platform << ::std::endl;
    os << "  Memory: " << info.total_memory_bytes / (1024 * 1024) << " MB" << ::std::endl;
    if (!info.linux_distro.empty()) {
        os << "  OS: " << info.linux_distro << ::std::endl;
    }
}

} // namespace mem_band

#endif // APU_IDENTIFIER_HPP
