#ifndef JSON_OUTPUT_HPP
#define JSON_OUTPUT_HPP

#include <string>
#include <vector>
#include <map>
#include <fstream>

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
