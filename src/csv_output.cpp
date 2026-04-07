#include "csv_output.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <iostream>

CSVOutput::CSVOutput(const std::string& filename)
    : file_(filename, std::ios::app)
    , header_written_(false)
    , is_open_(file_.is_open())
{
    // Check if file exists and has content
    if (file_.is_open()) {
        file_.seekp(0, std::ios::end);
        if (file_.tellp() == 0) {
            // New file, write header
            header_written_ = true;
            write_header();
        }
        file_.clear();
        file_.seekp(0, std::ios::end);
    } else {
        std::cerr << "Warning: Could not open file for writing: " << filename << "\n";
    }
}

CSVOutput::~CSVOutput() {
    close();
}

void CSVOutput::write_header() {
    if (!file_.is_open()) return;
    
    file_ << "timestamp,system_id,kernel,size_mib,data_type,iterations,bandwidth_gb_s,time_seconds,bytes_per_iter\n";
    file_.flush();
}

void CSVOutput::append(const BenchmarkResult& result) {
    if (!file_.is_open()) {
        std::cerr << "Warning: Cannot append to closed file\n";
        return;
    }
    
    // Write header if not yet written (in case file was created between open and append)
    if (!header_written_) {
        write_header();
        header_written_ = true;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << result.timestamp << ","
        << result.system_id << ","
        << result.kernel << ","
        << result.size_mib << ","
        << result.data_type << ","
        << result.iterations << ","
        << result.bandwidth_gb_s << ","
        << result.time_seconds << ","
        << result.bytes_per_iter << "\n";
    
    file_ << oss.str();
    file_.flush();
}

void CSVOutput::close() {
    if (file_.is_open()) {
        file_.close();
        is_open_ = false;
    }
}
