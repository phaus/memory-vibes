#ifndef LAYOUT_BUILDER_HPP
#define LAYOUT_BUILDER_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace mem_band {

struct SystemNode {
    std::string type;
    std::string name;
    std::map<std::string, std::string> metadata;
    
    SystemNode() = default;
    SystemNode(const std::string& t, const std::string& n) : type(t), name(n) {}
};

struct Connection {
    std::string type;
    double bandwidth_gb_s;
    
    Connection() = default;
    Connection(const std::string& t, double bw) : type(t), bandwidth_gb_s(bw) {}
};

struct SystemLayout {
    std::vector<SystemNode> nodes;
    std::vector<Connection> connections;
};

class LayoutBuilder {
public:
    LayoutBuilder() = default;
    
    LayoutBuilder& add_cpu(const std::string& model, int cores, int l3_cache_mb);
    LayoutBuilder& add_memory(long size_mb, int channels);
    LayoutBuilder& add_pci_device(const std::string& vendor, const std::string& device, 
                                   const std::string& device_type, int pcie_lanes = 16);
    LayoutBuilder& add_connection(size_t from_idx, size_t to_idx, 
                                   const std::string& type, double bandwidth_gb_s);
    
    SystemLayout build() &&;
    SystemLayout build() const&;

private:
    std::vector<SystemNode> nodes_;
    std::vector<Connection> connections_;
};

class LayoutFormatter {
public:
    virtual ~LayoutFormatter() = default;
    virtual std::string format(const SystemLayout& layout) = 0;
};

class TextFormatter : public LayoutFormatter {
public:
    std::string format(const SystemLayout& layout) override;
};

class MermaidFormatter : public LayoutFormatter {
public:
    std::string format(const SystemLayout& layout) override;
};

class JSONFormatter : public LayoutFormatter {
public:
    std::string format(const SystemLayout& layout) override;
};

} // namespace mem_band

#endif // LAYOUT_BUILDER_HPP
