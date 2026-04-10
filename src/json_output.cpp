#include "json_output.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>

JSONOutput::JSONOutput(const std::string& filename)
    : filename_(filename), header_written_(false), is_open_(false) {
    
    // Check if file exists and is empty
    std::ifstream check(filename_);
    if (check.is_open()) {
        check.seekg(0, std::ios::end);
        if (check.tellg() > 0) {
            header_written_ = true;
        }
        check.close();
    }
    
    file_.open(filename_, std::ios::out | std::ios::app);
    if (file_.is_open()) {
        is_open_ = true;
    }
}

JSONOutput::~JSONOutput() {
    close();
}

void JSONOutput::write_header() {
    if (!is_open_) return;
    
    if (!header_written_) {
        file_ << "[\n";
        header_written_ = true;
    }
}

void JSONOutput::append(const BenchmarkResult& result) {
    results_.push_back(result);
    
    if (!is_open_) return;
    
    write_header();
    
    // Add comma if not first entry
    if (results_.size() > 1) {
        file_ << ",\n";
    }
    
    file_ << "  {\n";
    file_ << "    \"timestamp\": " << result.timestamp << ",\n";
    file_ << "    \"system_id\": \"" << result.system_id << "\",\n";
    file_ << "    \"kernel\": \"" << result.kernel << "\",\n";
    file_ << "    \"size_mib\": " << result.size_mib << ",\n";
    file_ << "    \"data_type\": \"" << result.data_type << "\",\n";
    file_ << "    \"iterations\": " << result.iterations << ",\n";
    file_ << "    \"bandwidth_gb_s\": " << std::fixed << std::setprecision(6) 
          << result.bandwidth_gb_s << ",\n";
    file_ << "    \"time_seconds\": " << result.time_seconds << ",\n";
    file_ << "    \"bytes_per_iter\": " << result.bytes_per_iter << "\n";
    file_ << "  }";
}

void JSONOutput::close() {
    if (is_open_ && header_written_) {
        file_ << "\n]\n";
    }
    if (file_.is_open()) {
        file_.close();
        is_open_ = false;
    }
}

std::string JSONOutput::to_string() const {
    std::ostringstream oss;
    oss << "[\n";
    
    for (size_t i = 0; i < results_.size(); ++i) {
        const auto& r = results_[i];
        oss << "  {\n";
        oss << "    \"timestamp\": " << r.timestamp << ",\n";
        oss << "    \"system_id\": \"" << r.system_id << "\",\n";
        oss << "    \"kernel\": \"" << r.kernel << "\",\n";
        oss << "    \"size_mib\": " << r.size_mib << ",\n";
        oss << "    \"data_type\": \"" << r.data_type << "\",\n";
        oss << "    \"iterations\": " << r.iterations << ",\n";
        oss << "    \"bandwidth_gb_s\": " << std::fixed << std::setprecision(6) 
            << r.bandwidth_gb_s << ",\n";
        oss << "    \"time_seconds\": " << r.time_seconds << ",\n";
        oss << "    \"bytes_per_iter\": " << r.bytes_per_iter << "\n";
        oss << "  }";
        if (i < results_.size() - 1) {
            oss << ",";
        }
        oss << "\n";
    }
    
    oss << "]\n";
    return oss.str();
}
