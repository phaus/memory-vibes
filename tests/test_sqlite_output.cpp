// test_sqlite_output.cpp
// Unit tests for SQLite persistence layer

#include <gtest/gtest.h>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

#include "benchmark_result.hpp"
#include "sqlite_output.hpp"

#ifdef ENABLE_SQLITE

class SQLiteOutputTest : public ::testing::Test {
protected:
    std::string test_db_path;
    SQLiteOutput* output;
    
    void SetUp() override {
        test_db_path = "/tmp/test_mem_band_db_" + std::to_string(getpid()) + ".db";
        output = new SQLiteOutput(test_db_path);
    }
    
    void TearDown() override {
        delete output;
        fs::remove(test_db_path);
    }
};

TEST_F(SQLiteOutputTest, InitCreatesDatabaseAndSchema) {
    ASSERT_TRUE(fs::exists(test_db_path));
    ASSERT_NE(output, nullptr);
    
    sqlite3* db = nullptr;
    ASSERT_EQ(sqlite3_open(test_db_path.c_str(), &db), SQLITE_OK);
    
    const char* check_tables[] = {"benchmarks", "systems"};
    for (const char* table : check_tables) {
        sqlite3_stmt* stmt;
        std::string query = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + 
                           std::string(table) + "'";
        ASSERT_EQ(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr), SQLITE_OK);
        ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
}

TEST_F(SQLiteOutputTest, AppendBenchmarkResult) {
    BenchmarkResult result;
    result.timestamp = 1744465200000ULL;
    result.system_id = "test_system_001";
    result.kernel = "Copy";
    result.size_mib = 256;
    result.data_type = "float";
    result.iterations = 20;
    result.bandwidth_gb_s = 42.5;
    result.time_seconds = 1.5;
    result.bytes_per_iter = 1073741824;
    result.cpu_model = "Test CPU v1";
    result.os_name = "TestOS";
    result.os_version = "1.0";
    
    output->append(result);
    
    EXPECT_GT(output->get_result_count(), 0);
}

TEST_F(SQLiteOutputTest, MultipleAppends) {
    const char* system_id = "test_system_multi";
    
    for (int i = 0; i < 3; i++) {
        BenchmarkResult result;
        result.timestamp = 1744465200000ULL;
        result.system_id = system_id;
        result.kernel = (i == 0) ? "Copy" : (i == 1) ? "Triad" : "RandomRW";
        result.size_mib = 256;
        result.data_type = "float";
        result.iterations = 20;
        result.bandwidth_gb_s = 42.5 + i;
        result.time_seconds = 1.5 + i * 0.1;
        result.bytes_per_iter = 1073741824;
        result.cpu_model = "Test CPU";
        result.os_name = "TestOS";
        result.os_version = "1.0";
        
        output->append(result);
    }
    
    EXPECT_EQ(output->get_result_count(), 3);
}

#endif

int main(int argc, char** argv) {
#ifdef ENABLE_SQLITE
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
#else
    (void) argc;
    (void) argv;
    return 0;
#endif
}
