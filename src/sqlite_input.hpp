#ifndef SQLITE_INPUT_HPP
#define SQLITE_INPUT_HPP

#ifdef ENABLE_SQLITE

#include <string>
#include <vector>
#include <sqlite3.h>

#include "benchmark_result.hpp"

class SQLiteInput {
public:
    SQLiteInput(const std::string& db_path);
    ~SQLiteInput();
    
    // Query all benchmarks
    std::vector<BenchmarkResult> query_all() const;
    
    // Query benchmarks by system ID
    std::vector<BenchmarkResult> query_by_system_id(const std::string& system_id) const;
    
    // Query benchmarks by kernel name
    std::vector<BenchmarkResult> query_by_kernel(const std::string& kernel) const;
    
    // Query benchmarks by date range
    std::vector<BenchmarkResult> query_by_date_range(
        const std::string& start_date,
        const std::string& end_date
    ) const;
    
    // Search by pattern in multiple fields
    std::vector<BenchmarkResult> search(const std::string& pattern) const;
    
    // Get all system IDs
    std::vector<std::string> get_system_ids() const;
    
    // Get all unique kernel names
    std::vector<std::string> get_kernel_names() const;
    
    // Export to CSV
    bool export_csv(const std::string& file_path) const;
    
    // Export to JSON
    bool export_json(const std::string& file_path) const;
    
    // Get database path
    const std::string& get_db_path() const { return db_path_; }

private:
    std::string db_path_;
    sqlite3* db_;
    
    BenchmarkResult row_to_result(sqlite3_stmt* stmt) const;
    bool query_with_params(const char* sql, 
                           const std::vector<std::string>& params,
                           std::vector<BenchmarkResult>& results) const;
};

#endif // ENABLE_SQLITE

#endif // SQLITE_INPUT_HPP
