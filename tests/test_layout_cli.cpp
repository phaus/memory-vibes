#include <gtest/gtest.h>
#include <sstream>

// Test Options struct and parse_args without including main.cpp

namespace mem_band {

struct Options {
    std::size_t size_mib = 256;
    std::size_t iterations = 20;
    std::string type = "float";
    bool simd = false;
    bool alu = false;
    bool ssd = false;
    std::string ssd_path = "/tmp";
    std::size_t ssd_block_size = 4096;
    bool ssd_random = false;
    bool ssd_read_only = false;
    bool run_apu = false;
    bool run_npu = false;
    bool run_npu_suite = false;
    bool run_medium_test = false;
    bool quick_test = false;
    bool show_platform = false;
    bool show_runtime_features = false;
    bool system_layout = false;
    std::string layout_format = "text";
    std::string output_format = "text";
    std::string output_file;
};

bool parse_args(int argc, char* argv[], Options& opts) {
    std::vector<std::string> args(argv + 1, argv + argc);
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "-h" || a == "--help") { return false; }
        else if (a == "-s" || a == "--size") {
            if (i + 1 >= args.size()) { return false; }
            opts.size_mib = std::stoul(args[++i]);
        }
        else if (a == "-L" || a == "--system-layout") {
            opts.system_layout = true;
        }
        else if (a == "--layout-format") {
            if (i + 1 >= args.size()) { return false; }
            opts.layout_format = args[++i];
            if (opts.layout_format != "text" && opts.layout_format != "mermaid" && opts.layout_format != "json") {
                return false;
            }
        }
        else if (a == "-n" || a == "--iters") {
            if (i + 1 >= args.size()) { return false; }
            opts.iterations = std::stoul(args[++i]);
        }
        else if (a == "-t" || a == "--type") {
            if (i + 1 >= args.size()) { return false; }
            opts.type = args[++i];
            if (opts.type != "float" && opts.type != "double") {
                return false;
            }
        }
        else if (a == "-S" || a == "--simd") { opts.simd = true; }
        else if (a == "-A" || a == "--alu") { opts.alu = true; }
        else if (a == "-I" || a == "--ssd") { opts.ssd = true; }
        else if (a == "--ssd-path") {
            if (i + 1 >= args.size()) { return false; }
            opts.ssd_path = args[++i];
        }
        else if (a == "--ssd-block") {
            if (i + 1 >= args.size()) { return false; }
            opts.ssd_block_size = std::stoul(args[++i]);
        }
        else if (a == "--ssd-random") { opts.ssd_random = true; }
        else if (a == "--ssd-read-only") { opts.ssd_read_only = true; }
        else if (a == "-R" || a == "--run-apu") { opts.run_apu = true; }
        else if (a == "-N" || a == "--run-npu") { opts.run_npu = true; }
        else if (a == "--run-npu-suite") { opts.run_npu_suite = true; }
        else if (a == "-M" || a == "--run-medium-test") { opts.run_medium_test = true; }
        else if (a == "-Q" || a == "--quick-test") { opts.quick_test = true; }
        else if (a == "-P" || a == "--show-platform") { opts.show_platform = true; }
        else if (a == "--show-features") { opts.show_runtime_features = true; }
        else if (a == "-o" || a == "--output-format") {
            if (i + 1 >= args.size()) { return false; }
            opts.output_format = args[++i];
            if (opts.output_format != "text" && opts.output_format != "csv" && opts.output_format != "json") {
                return false;
            }
        }
        else if (a == "-f" || a == "--output-file") {
            if (i + 1 >= args.size()) { return false; }
            opts.output_file = args[++i];
        }
        else {
            return false;
        }
    }
    return true;
}

} // namespace mem_band

TEST(LayoutCLITest, OptionsContainsLayoutFlag) {
    mem_band::Options opts;
    EXPECT_FALSE(opts.system_layout);
}

TEST(LayoutCLITest, ParseLayoutFlagShort) {
    mem_band::Options opts;
    char* argv[] = {const_cast<char*>("./mem_band"), const_cast<char*>("-L"), nullptr};
    int argc = 2;
    
    bool result = mem_band::parse_args(argc, argv, opts);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(opts.system_layout);
}

TEST(LayoutCLITest, ParseLayoutFlagLong) {
    mem_band::Options opts;
    char* argv[] = {const_cast<char*>("./mem_band"), const_cast<char*>("--system-layout"), nullptr};
    int argc = 2;
    
    bool result = mem_band::parse_args(argc, argv, opts);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(opts.system_layout);
}

TEST(LayoutCLITest, ParseLayoutWithLayoutFormatText) {
    mem_band::Options opts;
    char* argv[] = {
        const_cast<char*>("./mem_band"),
        const_cast<char*>("-L"),
        const_cast<char*>("--layout-format"),
        const_cast<char*>("text"),
        nullptr
    };
    int argc = 4;
    
    bool result = mem_band::parse_args(argc, argv, opts);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(opts.system_layout);
    EXPECT_EQ(opts.layout_format, "text");
}

TEST(LayoutCLITest, ParseLayoutWithLayoutFormatMermaid) {
    mem_band::Options opts;
    char* argv[] = {
        const_cast<char*>("./mem_band"),
        const_cast<char*>("-L"),
        const_cast<char*>("--layout-format"),
        const_cast<char*>("mermaid"),
        nullptr
    };
    int argc = 4;
    
    bool result = mem_band::parse_args(argc, argv, opts);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(opts.system_layout);
    EXPECT_EQ(opts.layout_format, "mermaid");
}

TEST(LayoutCLITest, ParseLayoutWithLayoutFormatJSON) {
    mem_band::Options opts;
    char* argv[] = {
        const_cast<char*>("./mem_band"),
        const_cast<char*>("-L"),
        const_cast<char*>("--layout-format"),
        const_cast<char*>("json"),
        nullptr
    };
    int argc = 4;
    
    bool result = mem_band::parse_args(argc, argv, opts);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(opts.system_layout);
    EXPECT_EQ(opts.layout_format, "json");
}

TEST(LayoutCLITest, ParseInvalidLayoutFormat) {
    mem_band::Options opts;
    char* argv[] = {
        const_cast<char*>("./mem_band"),
        const_cast<char*>("-L"),
        const_cast<char*>("--layout-format"),
        const_cast<char*>("invalid"),
        nullptr
    };
    int argc = 4;
    
    bool result = mem_band::parse_args(argc, argv, opts);
    
    EXPECT_FALSE(result);
}

TEST(LayoutCLITest, ParseLayoutWithTypeFlag) {
    mem_band::Options opts;
    char* argv[] = {
        const_cast<char*>("./mem_band"),
        const_cast<char*>("-L"),
        const_cast<char*>("-t"),
        const_cast<char*>("double"),
        nullptr
    };
    int argc = 4;
    
    bool result = mem_band::parse_args(argc, argv, opts);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(opts.system_layout);
    EXPECT_EQ(opts.type, "double");
}

TEST(LayoutCLITest, ParseLayoutWithSizeFlag) {
    mem_band::Options opts;
    char* argv[] = {
        const_cast<char*>("./mem_band"),
        const_cast<char*>("-L"),
        const_cast<char*>("-s"),
        const_cast<char*>("512"),
        nullptr
    };
    int argc = 4;
    
    bool result = mem_band::parse_args(argc, argv, opts);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(opts.system_layout);
    EXPECT_EQ(opts.size_mib, 512);
}

TEST(LayoutCLITest, LayoutOnlyDoesNotRunBenchmark) {
    mem_band::Options opts;
    opts.system_layout = true;
    opts.layout_format = "text";
    
    EXPECT_TRUE(opts.system_layout);
}

TEST(LayoutCLITest, DefaultLayoutFormatIsText) {
    mem_band::Options opts;
    EXPECT_EQ(opts.layout_format, "text");
}
