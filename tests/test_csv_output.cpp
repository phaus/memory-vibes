#include <gtest/gtest.h>
#include <fstream>
#include "csv_output.hpp"

namespace {

class CSVOutputTest : public testing::Test {
protected:
    void SetUp() override {
        test_file_ = "/tmp/test_benchmark_output.csv";
        // Clean up any existing file
        std::remove(test_file_.c_str());
    }
    
    void TearDown() override {
        // Verify file exists after tests
        EXPECT_TRUE(std::ifstream(test_file_).good());
        // Cleanup
        std::remove(test_file_.c_str());
    }
    
    std::string test_file_;
};

TEST_F(CSVOutputTest, CreateFile) {
    CSVOutput csv(test_file_);
    EXPECT_TRUE(csv.is_open());
}

TEST_F(CSVOutputTest, WriteHeader) {
    CSVOutput csv(test_file_);
    std::ifstream file(test_file_);
    std::string line;
    std::getline(file, line);
    EXPECT_NE(line.find("kernel"), std::string::npos);
    EXPECT_NE(line.find("timestamp"), std::string::npos);
    EXPECT_NE(line.find("system_id"), std::string::npos);
}

TEST_F(CSVOutputTest, AppendResult) {
    BenchmarkResult result{
        1234567890,
        "abc123",
        "Copy",
        256,
        "float",
        20,
        3.23,
        0.62,
        2097152000
    };
    
    CSVOutput csv(test_file_);
    csv.append(result);
    
    std::ifstream file(test_file_);
    std::string line;
    std::getline(file, line); // Skip header
    std::getline(file, line);
    
    EXPECT_TRUE(line.find("abc123") != std::string::npos);
    EXPECT_TRUE(line.find("Copy") != std::string::npos);
    EXPECT_TRUE(line.find("256") != std::string::npos);
}

TEST_F(CSVOutputTest, MultipleResults) {
    CSVOutput csv(test_file_);
    
    for (int i = 0; i < 3; i++) {
        BenchmarkResult result{
            1234567890 + i,
            "abc123",
            ("Kernel" + std::to_string(i)),
            256 + i,
            "float",
            20,
            3.0 + i,
            0.62 + i,
            2097152000
        };
        csv.append(result);
    }
    
    std::ifstream file(test_file_);
    std::string line;
    int line_count = 0;
    while (std::getline(file, line)) {
        line_count++;
    }
    
    EXPECT_EQ(line_count, 4); // 1 header + 3 results
}

TEST_F(CSVOutputTest, HeaderNotDuplicate) {
    {
        CSVOutput csv1(test_file_);
        BenchmarkResult result1{
            1234567890,
            "abc123",
            "Copy",
            256,
            "float",
            20,
            3.23,
            0.62,
            2097152000
        };
        csv1.append(result1);
    }
    
    {
        CSVOutput csv2(test_file_);
        BenchmarkResult result2{
            1234567891,
            "abc123",
            "Triad",
            256,
            "float",
            20,
            3.23,
            0.93,
            3145728000
        };
        csv2.append(result2);
    }
    
    std::ifstream file(test_file_);
    std::string line;
    int header_count = 0;
    while (std::getline(file, line)) {
        if (line.find("kernel") != std::string::npos) {
            header_count++;
        }
    }
    
    EXPECT_EQ(header_count, 1);  // Header should only appear once
}

TEST_F(CSVOutputTest, CloseFile) {
    {
        CSVOutput csv(test_file_);
        EXPECT_TRUE(csv.is_open());
    }
    // After closing, file should still exist but stream is closed
    EXPECT_TRUE(std::ifstream(test_file_).good());
}

} // namespace
