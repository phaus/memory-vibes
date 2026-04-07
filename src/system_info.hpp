#ifndef SYSTEM_INFO_HPP
#define SYSTEM_INFO_HPP

#include <string>
#include <map>
#include <utility>
#include <thread>
#include <fstream>
#include <vector>
#include <cstdio>
/// macOS sysctl support
#if defined(__APPLE__)
#include <sys/sysctl.h>
#endif

struct SystemIdentifier {
    std::string cpu_model;
    int core_count;
    long memory_size_mb;
    std::string os_name;
    std::string os_version;
    std::string compiler_version;
    std::string platform;
    bool simd_enabled;
    std::string benchmark_version;
};

class SystemInfo {
public:
    static SystemIdentifier collect();
    static void print();
    static std::string generate_hash(const SystemIdentifier& id);

private:
    static std::string detect_cpu_model();
    static int detect_core_count();
    static long detect_memory_size();
    static std::pair<std::string, std::string> detect_os();
    static std::string detect_platform();
};

#endif // SYSTEM_INFO_HPP
