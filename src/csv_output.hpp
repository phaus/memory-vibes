#ifndef CSV_OUTPUT_HPP
#define CSV_OUTPUT_HPP

#include <string>
#include <fstream>
#include <vector>

struct BenchmarkResult {
    unsigned long long timestamp;
    std::string system_id;
    std::string kernel;
    unsigned long long size_mib;
    std::string data_type;
    unsigned long long iterations;
    double bandwidth_gb_s;
    double time_seconds;
    unsigned long long bytes_per_iter;
};

class CSVOutput {
public:
    CSVOutput(const std::string& filename);
    ~CSVOutput();
    
    // Append a benchmark result to the file
    void append(const BenchmarkResult& result);
    
    // Write CSV header (only if file is new)
    void write_header();
    
    // Close the file
    void close();

private:
    std::ofstream file_;
    bool header_written_;
    bool is_open_;
};

#endif // CSV_OUTPUT_HPP
