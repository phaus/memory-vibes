#define ENABLE_SQLITE
#include "sqlite_output.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

SQLiteOutput::SQLiteOutput(const std::string& db_path)
    : db_path_(db_path), db_(nullptr) {
    
    if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db_) << "\n";
        db_ = nullptr;
        return;
    }
    
    initialize_schema();
}

SQLiteOutput::~SQLiteOutput() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

void SQLiteOutput::initialize_schema() {
    if (!db_) return;
    
    const char* systems_table = R"(
        CREATE TABLE IF NOT EXISTS systems (
            system_id TEXT PRIMARY KEY,
            cpu_model TEXT,
            core_count INTEGER,
            memory_size_mb INTEGER,
            os_name TEXT,
            os_version TEXT,
            platform TEXT,
            created_at TEXT NOT NULL
        )
    )";
    
    const char* benchmarks_table = R"(
        CREATE TABLE IF NOT EXISTS benchmarks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT NOT NULL,
            system_id TEXT NOT NULL,
            kernel TEXT NOT NULL,
            size_mib INTEGER NOT NULL,
            data_type TEXT NOT NULL,
            iterations INTEGER NOT NULL,
            bandwidth_gb_s REAL NOT NULL,
            time_seconds REAL NOT NULL,
            bytes_per_iter INTEGER NOT NULL,
            cpu_model TEXT,
            os_name TEXT,
            os_version TEXT,
            FOREIGN KEY (system_id) REFERENCES systems(system_id)
        )
    )";
    
    const char* idx_timestamp = R"(
        CREATE INDEX IF NOT EXISTS idx_timestamp ON benchmarks(timestamp)
    )";
    
    const char* idx_system_id = R"(
        CREATE INDEX IF NOT EXISTS idx_system_id ON benchmarks(system_id)
    )";
    
    const char* idx_kernel = R"(
        CREATE INDEX IF NOT EXISTS idx_kernel ON benchmarks(kernel)
    )";
    
    char* err = nullptr;
    sqlite3_exec(db_, systems_table, nullptr, nullptr, &err);
    if (err) {
        std::cerr << "Error creating systems table: " << err << "\n";
        sqlite3_free(err);
        err = nullptr;
    }
    
    sqlite3_exec(db_, benchmarks_table, nullptr, nullptr, &err);
    if (err) {
        std::cerr << "Error creating benchmarks table: " << err << "\n";
        sqlite3_free(err);
        err = nullptr;
    }
    
    sqlite3_exec(db_, idx_timestamp, nullptr, nullptr, &err);
    sqlite3_free(err);
    err = nullptr;
    
    sqlite3_exec(db_, idx_system_id, nullptr, nullptr, &err);
    sqlite3_free(err);
    err = nullptr;
    
    sqlite3_exec(db_, idx_kernel, nullptr, nullptr, &err);
    sqlite3_free(err);
}

void SQLiteOutput::ensure_system_exists(BenchmarkResult& result) {
    if (!db_) return;
    
    const char* check_sql = R"(
        SELECT COUNT(*) FROM systems WHERE system_id = ?
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, check_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }
    
    sqlite3_bind_text(stmt, 1, result.system_id.c_str(), -1, SQLITE_STATIC);
    
    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = (sqlite3_column_int(stmt, 0) > 0);
    }
    sqlite3_finalize(stmt);
    
    if (!exists) {
        const char* insert_sql = R"(
            INSERT INTO systems (system_id, cpu_model, core_count, memory_size_mb, 
                                 os_name, os_version, platform, created_at)
            SELECT ?, ?, ?, ?, ?, ?, ?, ? WHERE NOT EXISTS (
                SELECT 1 FROM systems WHERE system_id = ?
            )
        )";
        
        sqlite3_stmt* insert_stmt;
        if (sqlite3_prepare_v2(db_, insert_sql, -1, &insert_stmt, nullptr) != SQLITE_OK) {
            return;
        }
        
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        auto epoch = now_ms.time_since_epoch();
        std::time_t timestamp = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S");
        std::string timestamp_str = ss.str();
        
        sqlite3_bind_text(insert_stmt, 1, result.system_id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(insert_stmt, 2, result.cpu_model.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(insert_stmt, 3, 0);  // core_count placeholder
        sqlite3_bind_int(insert_stmt, 4, 0);  // memory_size_mb placeholder
        sqlite3_bind_text(insert_stmt, 5, result.os_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(insert_stmt, 6, result.os_version.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(insert_stmt, 7, "", -1, SQLITE_STATIC);  // platform placeholder
        sqlite3_bind_text(insert_stmt, 8, timestamp_str.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(insert_stmt, 9, result.system_id.c_str(), -1, SQLITE_STATIC);
        
        sqlite3_step(insert_stmt);
        sqlite3_finalize(insert_stmt);
    }
}

void SQLiteOutput::append(const BenchmarkResult& result) {
    if (!db_) return;
    
    ensure_system_exists(const_cast<BenchmarkResult&>(result));
    
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    std::time_t timestamp = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S");
    std::string timestamp_str = ss.str();
    
    const char* sql = R"(
        INSERT INTO benchmarks (timestamp, system_id, kernel, size_mib, data_type, 
                                  iterations, bandwidth_gb_s, time_seconds, bytes_per_iter,
                                  cpu_model, os_name, os_version)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare insert statement: " << sqlite3_errmsg(db_) << "\n";
        return;
    }
    
    sqlite3_bind_text(stmt, 1, timestamp_str.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, result.system_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, result.kernel.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, static_cast<int>(result.size_mib));
    sqlite3_bind_text(stmt, 5, result.data_type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, static_cast<int>(result.iterations));
    sqlite3_bind_double(stmt, 7, result.bandwidth_gb_s);
    sqlite3_bind_double(stmt, 8, result.time_seconds);
    sqlite3_bind_int(stmt, 9, static_cast<int>(result.bytes_per_iter));
    sqlite3_bind_text(stmt, 10, result.cpu_model.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 11, result.os_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 12, result.os_version.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to execute insert: " << sqlite3_errmsg(db_) << "\n";
    }
    
    sqlite3_finalize(stmt);
}

std::size_t SQLiteOutput::get_result_count() const {
    if (!db_) return 0;
    
    const char* sql = "SELECT COUNT(*) FROM benchmarks";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }
    
    std::size_t count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = static_cast<std::size_t>(sqlite3_column_int(stmt, 0));
    }
    
    sqlite3_finalize(stmt);
    return count;
}
