#include "layout_builder.hpp"
#include <sstream>
#include <iomanip>

namespace mem_band {

LayoutBuilder& LayoutBuilder::add_cpu(const std::string& model, int cores, int l3_cache_mb) {
    SystemNode node("cpu", "CPU");
    node.metadata["model"] = model;
    node.metadata["cores"] = std::to_string(cores);
    node.metadata["l3_cache_mb"] = std::to_string(l3_cache_mb);
    nodes_.push_back(node);
    return *this;
}

LayoutBuilder& LayoutBuilder::add_memory(long size_mb, int channels) {
    SystemNode node("memory", "Memory");
    node.metadata["size_mb"] = std::to_string(size_mb);
    node.metadata["channels"] = std::to_string(channels);
    nodes_.push_back(node);
    return *this;
}

LayoutBuilder& LayoutBuilder::add_pci_device(const std::string& vendor, const std::string& device,
                                              const std::string& device_type, int pcie_lanes) {
    SystemNode node("pcie_device", device);
    node.metadata["vendor"] = vendor;
    node.metadata["device_type"] = device_type;
    node.metadata["pcie_lanes"] = std::to_string(pcie_lanes);
    nodes_.push_back(node);
    return *this;
}

LayoutBuilder& LayoutBuilder::add_connection(size_t from_idx, size_t to_idx,
                                              const std::string& type, double bandwidth_gb_s) {
    Connection conn(type, bandwidth_gb_s);
    connections_.push_back(conn);
    return *this;
}

SystemLayout LayoutBuilder::build() && {
    SystemLayout layout;
    layout.nodes = std::move(nodes_);
    layout.connections = std::move(connections_);
    nodes_.clear();
    connections_.clear();
    return layout;
}

SystemLayout LayoutBuilder::build() const& {
    SystemLayout layout;
    layout.nodes = nodes_;
    layout.connections = connections_;
    return layout;
}

std::string TextFormatter::format(const SystemLayout& layout) {
    std::ostringstream oss;
    
    oss << "┌─────────────────────────────────────────────────────────────┐\n";
    oss << "│                    System Layout                            │\n";
    oss << "├─────────────────────────────────────────────────────────────┤\n";
    
    for (const auto& node : layout.nodes) {
        if (node.type == "cpu") {
            oss << "│                                                             │\n";
            oss << "│  ┌─────────────┐                                           │\n";
            oss << "│  │    " << std::left << std::setw(13) << node.name << "│                                           │\n";
            oss << "│  │             │                                           │\n";
            if (node.metadata.count("model")) {
                oss << "│  │ Model: " << std::left << std::setw(9) 
                    << node.metadata.at("model").substr(0, 9) << "│                                           │\n";
            }
            if (node.metadata.count("cores")) {
                oss << "│  │ Cores: " << std::left << std::setw(9) 
                    << node.metadata.at("cores").substr(0, 9) << "│                                           │\n";
            }
            oss << "│  └─────────────┘                                           │\n";
        } else if (node.type == "memory") {
            oss << "│                                                             │\n";
            oss << "│  ┌─────────────┐                                           │\n";
            oss << "│  │    " << std::left << std::setw(13) << node.name << "│                                           │\n";
            oss << "│  │             │                                           │\n";
            if (node.metadata.count("size_mb")) {
                oss << "│  │ Size: " << std::left << std::setw(11) 
                    << node.metadata.at("size_mb").substr(0, 11) << "│                                           │\n";
            }
            if (node.metadata.count("channels")) {
                oss << "│  │ Chans: " << std::left << std::setw(10) 
                    << node.metadata.at("channels").substr(0, 10) << "│                                           │\n";
            }
            oss << "│  └─────────────┘                                           │\n";
        } else if (node.type == "pcie_device") {
            oss << "│                                                             │\n";
            oss << "│  ┌─────────────┐                                           │\n";
            oss << "│  │    " << std::left << std::setw(13) << node.name << "│                                           │\n";
            oss << "│  │             │                                           │\n";
            if (node.metadata.count("vendor")) {
                oss << "│  │ Vendor: " << std::left << std::setw(10) 
                    << node.metadata.at("vendor").substr(0, 10) << "│                                           │\n";
            }
            if (node.metadata.count("device_type")) {
                oss << "│  │ Type: " << std::left << std::setw(12) 
                    << node.metadata.at("device_type").substr(0, 12) << "│                                           │\n";
            }
            oss << "│  └─────────────┘                                           │\n";
        }
    }
    
    if (!layout.connections.empty()) {
        oss << "│                                                             │\n";
        oss << "│  Interconnects:                                             │\n";
        for (const auto& conn : layout.connections) {
            oss << "│    - " << conn.type << " @ " << conn.bandwidth_gb_s << " GB/s\n";
        }
    }
    
    oss << "│                                                             │\n";
    oss << "└─────────────────────────────────────────────────────────────┘\n";
    
    return oss.str();
}

std::string MermaidFormatter::format(const SystemLayout& layout) {
    std::ostringstream oss;
    
    oss << "```mermaid\n";
    oss << "flowchart TD\n";
    
    for (size_t i = 0; i < layout.nodes.size(); ++i) {
        const auto& node = layout.nodes[i];
        std::string node_id = "node" + std::to_string(i);
        
        if (node.type == "cpu") {
            oss << "    " << node_id << "[[" << node.name << "]]\n";
            oss << "    " << node_id << "[[" << node.name << "\\n";
            if (node.metadata.count("model")) {
                oss << "Model: " << node.metadata.at("model") << "\\n";
            }
            if (node.metadata.count("cores")) {
                oss << "Cores: " << node.metadata.at("cores");
            }
            oss << "]]\n";
        } else if (node.type == "memory") {
            oss << "    " << node_id << "[[" << node.name << "\\n";
            if (node.metadata.count("size_mb")) {
                oss << "Size: " << node.metadata.at("size_mb") << " MB\\n";
            }
            if (node.metadata.count("channels")) {
                oss << "Channels: " << node.metadata.at("channels");
            }
            oss << "]]\n";
        } else if (node.type == "pcie_device") {
            oss << "    " << node_id << "[[" << node.name << "\\n";
            if (node.metadata.count("vendor")) {
                oss << "Vendor: " << node.metadata.at("vendor") << "\\n";
            }
            if (node.metadata.count("device_type")) {
                oss << "Type: " << node.metadata.at("device_type");
            }
            oss << "]]\n";
        }
    }
    
    for (const auto& conn : layout.connections) {
        oss << "    " << "node0" << " -->|\"" << conn.type << " " << conn.bandwidth_gb_s << " GB/s\"| " << "node1" << "\n";
    }
    
    oss << "```\n";
    
    return oss.str();
}

std::string JSONFormatter::format(const SystemLayout& layout) {
    std::ostringstream oss;
    
    oss << "{\n";
    oss << "  \"nodes\": [\n";
    
    for (size_t i = 0; i < layout.nodes.size(); ++i) {
        const auto& node = layout.nodes[i];
        oss << "    {\n";
        oss << "      \"type\": \"" << node.type << "\",\n";
        oss << "      \"name\": \"" << node.name << "\",\n";
        oss << "      \"metadata\": {\n";
        
        size_t j = 0;
        for (const auto& [key, value] : node.metadata) {
            oss << "        \"" << key << "\": \"" << value << "\"";
            if (j < node.metadata.size() - 1) oss << ",";
            oss << "\n";
            ++j;
        }
        
        oss << "      }\n";
        oss << "    }";
        if (i < layout.nodes.size() - 1) oss << ",";
        oss << "\n";
    }
    
    oss << "  ],\n";
    oss << "  \"connections\": [\n";
    
    for (size_t i = 0; i < layout.connections.size(); ++i) {
        const auto& conn = layout.connections[i];
        oss << "    {\n";
        oss << "      \"type\": \"" << conn.type << "\",\n";
        oss << "      \"bandwidth_gb_s\": " << std::fixed << std::setprecision(1) << conn.bandwidth_gb_s << "\n";
        oss << "    }";
        if (i < layout.connections.size() - 1) oss << ",";
        oss << "\n";
    }
    
    oss << "  ]\n";
    oss << "}\n";
    
    return oss.str();
}

} // namespace mem_band
