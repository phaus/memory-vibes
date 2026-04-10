#ifndef JSON_OUTPUT_HPP
#define JSON_OUTPUT_HPP

#include <string>
#include <vector>
#include <map>
#include <fstream>

#include "benchmark_result.hpp"

class JSONOutput {
public:
    JSONOutput(const std::string& filename);
    ~JSONOutput();
    
    // Append a benchmark result to the file
    void append(const BenchmarkResult& result);
    
    // Write JSON header/array start (only if file is new)
    void write_header();
    
    // Close the file and write array end
    void close();

    // Get the current data as a JSON string (for in-memory operations)
    std::string to_string() const;

private:
    std::string filename_;
    std::ofstream file_;
    bool header_written_;
    bool is_open_;
    std::vector<BenchmarkResult> results_;
};

#endif // JSON_OUTPUT_HPP
