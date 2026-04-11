#define ENABLE_SQLITE
#include "sqlite_input.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

SQLiteInput::SQLiteInput(const std::string& db_path)
    : db_path_(db_path), db_(nullptr) {
    
    if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db_) << "\n";
        db_ = nullptr;
    }
}

SQLiteInput::~SQLiteInput() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

BenchmarkResult SQLiteInput::row_to_result(sqlite3_stmt* stmt) const {
    BenchmarkResult result;
    result.timestamp = 0;
    
    int col = 0;
    
    const char* timestamp_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
    if (timestamp_str) {
        std::string ts(timestamp_str);
        std::istringstream iss(ts);
        std::tm tm;
        iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        std::time_t time = std::mktime(&tm);
        result.timestamp = static_cast<unsigned long long>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::from_time_t(time).time_since_epoch()
            ).count()
        );
    }
    
    const char* system_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
    if (system_id) result.system_id = system_id;
    
    const char* kernel = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
    if (kernel) result.kernel = kernel;
    
    result.size_mib = static_cast<unsigned long long>(sqlite3_column_int(stmt, col++));
    result.data_type = sqlite3_column_text(stmt, col++) ? 
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, col - 1)) : "";
    
    result.iterations = static_cast<unsigned long long>(sqlite3_column_int(stmt, col++));
    result.bandwidth_gb_s = sqlite3_column_double(stmt, col++);
    result.time_seconds = sqlite3_column_double(stmt, col++);
    result.bytes_per_iter = static_cast<unsigned long long>(sqlite3_column_int64(stmt, col++));
    
    const char* cpu_model = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
    if (cpu_model) result.cpu_model = cpu_model;
    
    const char* os_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
    if (os_name) result.os_name = os_name;
    
    const char* os_version = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
    if (os_version) result.os_version = os_version;
    
    return result;
}

bool SQLiteInput::query_with_params(const char* sql, 
                                     const std::vector<std::string>& params,
                                     std::vector<BenchmarkResult>& results) const {
    if (!db_) return false;
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << "\n";
        return false;
    }
    
    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_STATIC);
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        results.push_back(row_to_result(stmt));
    }
    
    sqlite3_finalize(stmt);
    return true;
}

std::vector<BenchmarkResult> SQLiteInput::query_all() const {
    std::vector<BenchmarkResult> results;
    
    const char* sql = R"(
        SELECT timestamp, system_id, kernel, size_mib, data_type, iterations,
               bandwidth_gb_s, time_seconds, bytes_per_iter, cpu_model, os_name, os_version
        FROM benchmarks
        ORDER BY timestamp DESC
    )";
    
    query_with_params(sql, {}, results);
    return results;
}

std::vector<BenchmarkResult> SQLiteInput::query_by_system_id(const std::string& system_id) const {
    std::vector<BenchmarkResult> results;
    
    const char* sql = R"(
        SELECT timestamp, system_id, kernel, size_mib, data_type, iterations,
               bandwidth_gb_s, time_seconds, bytes_per_iter, cpu_model, os_name, os_version
        FROM benchmarks
        WHERE system_id = ?
        ORDER BY timestamp DESC
    )";
    
    query_with_params(sql, {system_id}, results);
    return results;
}

std::vector<BenchmarkResult> SQLiteInput::query_by_kernel(const std::string& kernel) const {
    std::vector<BenchmarkResult> results;
    
    const char* sql = R"(
        SELECT timestamp, system_id, kernel, size_mib, data_type, iterations,
               bandwidth_gb_s, time_seconds, bytes_per_iter, cpu_model, os_name, os_version
        FROM benchmarks
        WHERE kernel = ?
        ORDER BY timestamp DESC
    )";
    
    query_with_params(sql, {kernel}, results);
    return results;
}

std::vector<BenchmarkResult> SQLiteInput::query_by_date_range(
    const std::string& start_date,
    const std::string& end_date
) const {
    std::vector<BenchmarkResult> results;
    
    const char* sql = R"(
        SELECT timestamp, system_id, kernel, size_mib, data_type, iterations,
               bandwidth_gb_s, time_seconds, bytes_per_iter, cpu_model, os_name, os_version
        FROM benchmarks
        WHERE timestamp >= ? AND timestamp <= ?
        ORDER BY timestamp DESC
    )";
    
    query_with_params(sql, {start_date, end_date}, results);
    return results;
}

std::vector<BenchmarkResult> SQLiteInput::search(const std::string& pattern) const {
    std::vector<BenchmarkResult> results;
    
    std::string pattern_param = "%" + pattern + "%";
    
    const char* sql = R"(
        SELECT timestamp, system_id, kernel, size_mib, data_type, iterations,
               bandwidth_gb_s, time_seconds, bytes_per_iter, cpu_model, os_name, os_version
        FROM benchmarks
        WHERE system_id LIKE ? OR kernel LIKE ? OR cpu_model LIKE ? OR os_name LIKE ?
        ORDER BY timestamp DESC
    )";
    
    query_with_params(sql, {pattern_param, pattern_param, pattern_param, pattern_param}, results);
    return results;
}

std::vector<std::string> SQLiteInput::get_system_ids() const {
    std::vector<std::string> system_ids;
    
    if (!db_) return system_ids;
    
    const char* sql = "SELECT DISTINCT system_id FROM systems ORDER BY system_id";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << "\n";
        return system_ids;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (text) system_ids.push_back(text);
    }
    
    sqlite3_finalize(stmt);
    return system_ids;
}

std::vector<std::string> SQLiteInput::get_kernel_names() const {
    std::vector<std::string> kernel_names;
    
    if (!db_) return kernel_names;
    
    const char* sql = "SELECT DISTINCT kernel FROM benchmarks ORDER BY kernel";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << "\n";
        return kernel_names;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (text) kernel_names.push_back(text);
    }
    
    sqlite3_finalize(stmt);
    return kernel_names;
}

bool SQLiteInput::export_csv(const std::string& file_path) const {
    std::ofstream outfile(file_path);
    if (!outfile.is_open()) {
        std::cerr << "Failed to open file for writing: " << file_path << "\n";
        return false;
    }
    
    outfile << "timestamp,system_id,kernel,size_mib,data_type,iterations,"
            << "bandwidth_gb_s,time_seconds,bytes_per_iter,cpu_model,os_name,os_version\n";
    
    const char* sql = R"(
        SELECT timestamp, system_id, kernel, size_mib, data_type, iterations,
               bandwidth_gb_s, time_seconds, bytes_per_iter, cpu_model, os_name, os_version
        FROM benchmarks
        ORDER BY timestamp DESC
    )";
    
    sqlite3_stmt* stmt;
    if (!db_ || sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << "\n";
        return false;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::ostringstream row;
        
        int col = 0;
        const char* timestamp_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
        row << (timestamp_str ? timestamp_str : "") << ",";
        
        const char* system_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
        row << (system_id ? system_id : "") << ",";
        
        const char* kernel = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
        row << (kernel ? kernel : "") << ",";
        
        row << sqlite3_column_int(stmt, col++) << ",";
        
        const char* data_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
        row << (data_type ? data_type : "") << ",";
        
        row << sqlite3_column_int(stmt, col++) << ",";
        row << std::setprecision(6) << sqlite3_column_double(stmt, col++) << ",";
        row << std::setprecision(6) << sqlite3_column_double(stmt, col++) << ",";
        row << sqlite3_column_int64(stmt, col++) << ",";
        
        const char* cpu_model = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
        row << (cpu_model ? cpu_model : "") << ",";
        
        const char* os_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
        row << (os_name ? os_name : "") << ",";
        
        const char* os_version = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
        row << (os_version ? os_version : "");
        
        outfile << row.str() << "\n";
    }
    
    sqlite3_finalize(stmt);
    outfile.close();
    return true;
}

bool SQLiteInput::export_json(const std::string& file_path) const {
    std::ofstream outfile(file_path);
    if (!outfile.is_open()) {
        std::cerr << "Failed to open file for writing: " << file_path << "\n";
        return false;
    }
    
    const char* sql = R"(
        SELECT timestamp, system_id, kernel, size_mib, data_type, iterations,
               bandwidth_gb_s, time_seconds, bytes_per_iter, cpu_model, os_name, os_version
        FROM benchmarks
        ORDER BY timestamp DESC
    )";
    
    sqlite3_stmt* stmt;
    if (!db_ || sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << "\n";
        return false;
    }
    
    outfile << "[\n";
    bool first = true;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) outfile << ",\n";
        first = false;
        
        int col = 0;
        const char* timestamp_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
        
        const char* system_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
        
        const char* kernel = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
        
        int size_mib = sqlite3_column_int(stmt, col++);
        
        const char* data_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
        
        int iterations = sqlite3_column_int(stmt, col++);
        
        double bandwidth_gb_s = sqlite3_column_double(stmt, col++);
        double time_seconds = sqlite3_column_double(stmt, col++);
        
        long long bytes_per_iter = sqlite3_column_int64(stmt, col++);
        
        const char* cpu_model = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
        
        const char* os_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
        
        const char* os_version = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col++));
        
        outfile << "  {\n";
        outfile << "    \"timestamp\": \"" << (timestamp_str ? timestamp_str : "") << "\",\n";
        outfile << "    \"system_id\": \"" << (system_id ? system_id : "") << "\",\n";
        outfile << "    \"kernel\": \"" << (kernel ? kernel : "") << "\",\n";
        outfile << "    \"size_mib\": " << size_mib << ",\n";
        outfile << "    \"data_type\": \"" << (data_type ? data_type : "") << "\",\n";
        outfile << "    \"iterations\": " << iterations << ",\n";
        outfile << "    \"bandwidth_gb_s\": " << std::setprecision(10) << bandwidth_gb_s << ",\n";
        outfile << "    \"time_seconds\": " << std::setprecision(10) << time_seconds << ",\n";
        outfile << "    \"bytes_per_iter\": " << bytes_per_iter << ",\n";
        outfile << "    \"cpu_model\": \"" << (cpu_model ? cpu_model : "") << "\",\n";
        outfile << "    \"os_name\": \"" << (os_name ? os_name : "") << "\",\n";
        outfile << "    \"os_version\": \"" << (os_version ? os_version : "") << "\"\n";
        outfile << "  }";
    }
    
    outfile << "\n]\n";
    sqlite3_finalize(stmt);
    outfile.close();
    return true;
}
