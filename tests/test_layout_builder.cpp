#include <gtest/gtest.h>
#include "layout_builder.hpp"

using namespace mem_band;

class LayoutBuilderTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(LayoutBuilderTest, TestSystemNode) {
    SystemNode node("cpu", "CPU");
    node.metadata["model"] = "AMD Ryzen 9";
    node.metadata["cores"] = "16";
    node.metadata["l3_cache_mb"] = "64";
    
    EXPECT_EQ(node.type, "cpu");
    EXPECT_EQ(node.name, "CPU");
    EXPECT_EQ(node.metadata["model"], "AMD Ryzen 9");
    EXPECT_EQ(node.metadata["cores"], "16");
    EXPECT_EQ(node.metadata["l3_cache_mb"], "64");
}

TEST_F(LayoutBuilderTest, TestConnection) {
    Connection conn("PCIe", 32.0);
    EXPECT_EQ(conn.type, "PCIe");
    EXPECT_DOUBLE_EQ(conn.bandwidth_gb_s, 32.0);
}

TEST_F(LayoutBuilderTest, TestLayoutBuilderBuildCPU) {
    LayoutBuilder builder;
    
    builder.add_cpu("Intel Core i9", 8, 16);
    
    auto layout = builder.build();
    
    EXPECT_FALSE(layout.nodes.empty());
    EXPECT_EQ(layout.nodes[0].name, "CPU");
    EXPECT_EQ(layout.nodes[0].type, "cpu");
    EXPECT_EQ(layout.nodes[0].metadata.at("model"), "Intel Core i9");
}

TEST_F(LayoutBuilderTest, TestLayoutBuilderBuildMemory) {
    LayoutBuilder builder;
    
    builder.add_memory(32 * 1024, 4);
    
    auto layout = builder.build();
    
    EXPECT_FALSE(layout.nodes.empty());
    auto mem_node = std::find_if(layout.nodes.begin(), layout.nodes.end(),
        [](const SystemNode& node) { return node.type == "memory"; });
    EXPECT_NE(mem_node, layout.nodes.end());
    EXPECT_EQ(mem_node->metadata.at("size_mb"), "32768");
}

TEST_F(LayoutBuilderTest, TestLayoutBuilderBuildPCIe) {
    LayoutBuilder builder;
    
    builder.add_pci_device("AMD", "Radeon RX 6800", "GPU", 16);
    
    auto layout = builder.build();
    
    EXPECT_FALSE(layout.nodes.empty());
    auto pci_node = std::find_if(layout.nodes.begin(), layout.nodes.end(),
        [](const SystemNode& node) { return node.type == "pcie_device"; });
    EXPECT_NE(pci_node, layout.nodes.end());
    EXPECT_EQ(pci_node->name, "Radeon RX 6800");
}

TEST_F(LayoutBuilderTest, TestLayoutBuilderComplex) {
    LayoutBuilder builder;
    
    builder.add_cpu("AMD Ryzen 9", 16, 64);
    builder.add_memory(128 * 1024, 2);
    builder.add_pci_device("NVIDIA", "RTX 3090", "GPU", 16);
    builder.add_connection(0, 1, "NUMA", 100.0);
    builder.add_connection(1, 2, "PCIe", 32.0);
    
    auto layout = builder.build();
    
    EXPECT_EQ(layout.nodes.size(), 3);
    EXPECT_EQ(layout.connections.size(), 2);
    
    auto cpu_node = std::find_if(layout.nodes.begin(), layout.nodes.end(),
        [](const SystemNode& node) { return node.type == "cpu"; });
    auto mem_node = std::find_if(layout.nodes.begin(), layout.nodes.end(),
        [](const SystemNode& node) { return node.type == "memory"; });
    auto pci_node = std::find_if(layout.nodes.begin(), layout.nodes.end(),
        [](const SystemNode& node) { return node.type == "pcie_device"; });
    
    EXPECT_NE(cpu_node, layout.nodes.end());
    EXPECT_NE(mem_node, layout.nodes.end());
    EXPECT_NE(pci_node, layout.nodes.end());
}

TEST_F(LayoutBuilderTest, TestTextFormatter) {
    LayoutBuilder builder;
    
    builder.add_cpu("AMD Ryzen 9", 16, 64);
    builder.add_memory(128 * 1024, 2);
    
    auto layout = builder.build();
    TextFormatter formatter;
    std::string output = formatter.format(layout);
    
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("AMD Ryzen"), std::string::npos);
    EXPECT_NE(output.find("CPU"), std::string::npos);
}

TEST_F(LayoutBuilderTest, TestMermaidFormatter) {
    LayoutBuilder builder;
    
    builder.add_cpu("Intel Core i7", 8, 16);
    
    auto layout = builder.build();
    MermaidFormatter formatter;
    std::string output = formatter.format(layout);
    
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("flowchart"), std::string::npos);
}

TEST_F(LayoutBuilderTest, TestJSONFormatter) {
    LayoutBuilder builder;
    
    builder.add_cpu("ARM Cortex-X1", 8, 32);
    
    auto layout = builder.build();
    JSONFormatter formatter;
    std::string output = formatter.format(layout);
    
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("\"nodes\""), std::string::npos);
    EXPECT_NE(output.find("CPU"), std::string::npos);
}

TEST_F(LayoutBuilderTest, TestEmptyLayout) {
    LayoutBuilder builder;
    auto layout = builder.build();
    
    EXPECT_TRUE(layout.nodes.empty());
    EXPECT_TRUE(layout.connections.empty());
}
