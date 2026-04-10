#ifndef CSV_OUTPUT_HPP
#define CSV_OUTPUT_HPP

#include <string>
#include <fstream>
#include <vector>

#include "benchmark_result.hpp"

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

    // Query whether the underlying file stream is open
    bool is_open() const { return is_open_; }

private:
    std::ofstream file_;
    bool header_written_;
    bool is_open_;
};

#endif // CSV_OUTPUT_HPP
