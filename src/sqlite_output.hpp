#ifndef SQLITE_OUTPUT_HPP
#define SQLITE_OUTPUT_HPP

#ifdef ENABLE_SQLITE

#include <string>
#include <sqlite3.h>

#include "benchmark_result.hpp"

class SQLiteOutput {
public:
    SQLiteOutput(const std::string& db_path);
    ~SQLiteOutput();
    
    // Append a benchmark result to the database
    void append(const BenchmarkResult& result);
    
    // Get database path
    const std::string& get_db_path() const { return db_path_; }
    
    // Get number of stored results
    std::size_t get_result_count() const;

private:
    std::string db_path_;
    sqlite3* db_;
    
    void initialize_schema();
    void ensure_system_exists(BenchmarkResult& result);
};

#endif // ENABLE_SQLITE

#endif // SQLITE_OUTPUT_HPP
