#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>

#include "apu_identifier.hpp"

int main() {
    // Test system identifier collection
    mem_band::APUSystemInfo info = mem_band::collect_apu_system_info();
    
    // Basic validation: should have some identifier populated
    assert(!info.cpu_model.empty() || info.cpu_model == "<unknown>");
    assert(info.total_memory_bytes > 0);
    
    // Test CPU model string validation
    if (info.cpu_model != "<unknown>") {
        assert(!info.cpu_model.empty());
        // CPU model should contain letters or numbers
        bool has_alphanumeric = false;
        for (char c : info.cpu_model) {
            if (std::isalnum(static_cast<unsigned char>(c))) {
                has_alphanumeric = true;
                break;
            }
        }
        assert(has_alphanumeric);
    }
    
    // Test memory size validation (should be at least 1MB on modern systems)
    assert(info.total_memory_bytes >= 1024 * 1024);
    
    // Test Strix Point detection if available
    bool is_strix_point = info.cpu_model.find("Strix") != std::string::npos ||
                          info.cpu_model.find("HX 370") != std::string::npos ||
                          info.cpu_model.find("HS 370") != std::string::npos;
    
    // Test Halo detection if available  
    bool is_halo = info.cpu_model.find("Halo") != std::string::npos;
    
    // Output for manual verification
    ::std::cout << "Collected System Info:\n";
    ::std::cout << "  CPU: " << info.cpu_model << ::std::endl;
    ::std::cout << "  Memory: " << info.total_memory_bytes / (1024 * 1024) << " MB" << ::std::endl;
    ::std::cout << " Strix Point: " << (is_strix_point ? "yes" : "no") << ::std::endl;
    ::std::cout << "  Halo: " << (is_halo ? "yes" : "no") << ::std::endl;
    
    return 0;
}
