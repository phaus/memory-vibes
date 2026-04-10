#include <gtest/gtest.h>
#include "json_output.hpp"
#include <fstream>
#include <sstream>
#include <system_error>

namespace mem_band {

TEST(JSONOutputTest, BasicWrite) {
    const std::string test_file = "/tmp/test_json_output.json";
    
    // Remove file if exists
    std::remove(test_file.c_str());
    
    JSONOutput json(test_file);
    json.write_header();
    
    BenchmarkResult result{};
    result.timestamp = 1234567890;
    result.system_id = "test-system";
    result.kernel = "Copy";
    result.size_mib = 256;
    result.data_type = "float";
    result.iterations = 20;
    result.bandwidth_gb_s = 3.14159;
    result.time_seconds = 0.5;
    result.bytes_per_iter = 2097152;
    
    json.append(result);
    json.close();
    
    // Read back and verify
    std::ifstream file(test_file);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    EXPECT_NE(content.find("\"kernel\": \"Copy\""), std::string::npos);
    EXPECT_NE(content.find("\"system_id\": \"test-system\""), std::string::npos);
    
    std::remove(test_file.c_str());
}

TEST(JSONOutputTest, MultipleResults) {
    const std::string test_file = "/tmp/test_json_multiple.json";
    
    // Remove file if exists
    std::remove(test_file.c_str());
    
    JSONOutput json(test_file);
    json.write_header();
    
    for (int i = 0; i < 3; ++i) {
        BenchmarkResult result{};
        result.timestamp = 1234567890 + i;
        result.system_id = "test-system-" + std::to_string(i);
        result.kernel = "Copy";
        result.size_mib = 256 + i;
        result.data_type = "float";
        result.iterations = 20;
        result.bandwidth_gb_s = 3.14159 * (i + 1);
        result.time_seconds = 0.5;
        result.bytes_per_iter = 2097152;
        
        json.append(result);
    }
    
    json.close();
    
    // Read back and verify
    std::ifstream file(test_file);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    EXPECT_NE(content.find("\"system_id\": \"test-system-0\""), std::string::npos);
    EXPECT_NE(content.find("\"system_id\": \"test-system-1\""), std::string::npos);
    EXPECT_NE(content.find("\"system_id\": \"test-system-2\""), std::string::npos);
    
    std::remove(test_file.c_str());
}

TEST(JSONOutputTest, ToString) {
    JSONOutput json("/tmp/nonexistent.json");
    
    for (int i = 0; i < 2; ++i) {
        BenchmarkResult result{};
        result.timestamp = 1234567890 + i;
        result.system_id = "sys-" + std::to_string(i);
        result.kernel = "Copy";
        result.size_mib = 256;
        result.data_type = "float";
        result.iterations = 20;
        result.bandwidth_gb_s = 3.14159;
        result.time_seconds = 0.5;
        result.bytes_per_iter = 2097152;
        
        json.append(result);
    }
    
    std::string content = json.to_string();
    
    EXPECT_NE(content.find("\"system_id\": \"sys-0\""), std::string::npos);
    EXPECT_NE(content.find("\"system_id\": \"sys-1\""), std::string::npos);
    EXPECT_EQ(content[0], '[');
    EXPECT_EQ(content[content.length() - 2], ']');
}

}