// test_sqlite_input.cpp
// Unit tests for SQLite database querying and export functionality

#include <gtest/gtest.h>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

#include "benchmark_result.hpp"
#include "sqlite_output.hpp"
#include "sqlite_input.hpp"

#ifdef ENABLE_SQLITE

class SQLiteInputTest : public ::testing::Test {
protected:
    std::string test_db_path;
    SQLiteOutput* output;
    SQLiteInput* input;
    
    void SetUp() override {
        test_db_path = "/tmp/test_mem_band_input_" + std::to_string(getpid()) + ".db";
        
        // Create database with some test data
        output = new SQLiteOutput(test_db_path);
        
        // Insert several test results
        for (int i = 0; i < 5; i++) {
            BenchmarkResult result;
            result.timestamp = 1744465200000ULL + i * 1000;
            result.system_id = "test_system_" + std::to_string(i % 2);
            result.kernel = (i == 0 || i == 3) ? "Copy" : (i == 1 || i == 4) ? "Triad" : "RandomRW";
            result.size_mib = 256;
            result.data_type = "float";
            result.iterations = 20;
            result.bandwidth_gb_s = 50.0 + i * 2;
            result.time_seconds = 1.0 + i * 0.1;
            result.bytes_per_iter = 1073741824;
            result.cpu_model = "Test CPU v1";
            result.os_name = "TestOS";
            result.os_version = "1.0";
            
            output->append(result);
        }
        
        delete output;
        
        // Open for reading
        input = new SQLiteInput(test_db_path);
    }
    
    void TearDown() override {
        delete input;
        fs::remove(test_db_path);
    }
};

TEST_F(SQLiteInputTest, QueryAllReturnsResults) {
    auto results = input->query_all();
    EXPECT_EQ(results.size(), 5);
    EXPECT_EQ(results[0].kernel, "RandomRW");
    EXPECT_EQ(results[4].kernel, "Copy");
}

TEST_F(SQLiteInputTest, QueryBySystemId) {
    auto results = input->query_by_system_id("test_system_0");
    EXPECT_EQ(results.size(), 3);
}

TEST_F(SQLiteInputTest, QueryByKernel) {
    auto results = input->query_by_kernel("Copy");
    EXPECT_EQ(results.size(), 2);
    for (const auto& r : results) {
        EXPECT_EQ(r.kernel, "Copy");
    }
}

TEST_F(SQLiteInputTest, SearchByPattern) {
    auto results = input->search("test_system_0");
    EXPECT_EQ(results.size(), 3);
}

TEST_F(SQLiteInputTest, SearchByKernelPattern) {
    auto results = input->search("Copy");
    EXPECT_EQ(results.size(), 2);
}

TEST_F(SQLiteInputTest, GetSystemIds) {
    auto system_ids = input->get_system_ids();
    EXPECT_EQ(system_ids.size(), 2);
    EXPECT_NE(std::find(system_ids.begin(), system_ids.end(), "test_system_0"), system_ids.end());
    EXPECT_NE(std::find(system_ids.begin(), system_ids.end(), "test_system_1"), system_ids.end());
}

TEST_F(SQLiteInputTest, GetKernelNames) {
    auto kernel_names = input->get_kernel_names();
    EXPECT_EQ(kernel_names.size(), 3);
    EXPECT_NE(std::find(kernel_names.begin(), kernel_names.end(), "Copy"), kernel_names.end());
    EXPECT_NE(std::find(kernel_names.begin(), kernel_names.end(), "Triad"), kernel_names.end());
    EXPECT_NE(std::find(kernel_names.begin(), kernel_names.end(), "RandomRW"), kernel_names.end());
}

TEST_F(SQLiteInputTest, ExportCsv) {
    std::string csv_path = test_db_path + ".csv";
    bool success = input->export_csv(csv_path);
    EXPECT_TRUE(success);
    
    std::ifstream file(csv_path);
    EXPECT_TRUE(file.is_open());
    
    std::string line;
    std::getline(file, line);
    EXPECT_FALSE(line.empty());
    EXPECT_NE(line.find("timestamp"), std::string::npos);
    
    file.close();
    fs::remove(csv_path);
}

TEST_F(SQLiteInputTest, ExportJson) {
    std::string json_path = test_db_path + ".json";
    bool success = input->export_json(json_path);
    EXPECT_TRUE(success);
    
    std::ifstream file(json_path);
    EXPECT_TRUE(file.is_open());
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    EXPECT_NE(content.find("\"timestamp\""), std::string::npos);
    EXPECT_NE(content.find("\"system_id\""), std::string::npos);
    
    file.close();
    fs::remove(json_path);
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
