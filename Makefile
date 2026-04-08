# Makefile for Memory Bandwidth Benchmark
# CMake-based build system with Makefile convenience targets

.PHONY: all build clean test lint help debug release simd docs

CMAKE_FLAGS ?= -DCMAKE_BUILD_TYPE=Release
BUILD_DIR = build

all: build

build:
	@echo "Building project..."
	@mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake .. $(CMAKE_FLAGS) && cmake --build .
	@echo "Build complete. Executable: $(BUILD_DIR)/mem_band"

debug:
	@echo "Building in Debug mode..."
	@rm -rf $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build .
	@echo "Debug build complete."

release:
	@echo "Building in Release mode..."
	@rm -rf $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build .
	@echo "Release build complete."

simd:
	@echo "Building with SIMD support..."
	@rm -rf $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_SIMD=ON && cmake --build .
	@echo "SIMD build complete."

clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)
	@echo "Clean complete."

test: build
	@echo "Running tests..."
	cd $(BUILD_DIR) && ctest --output-on-failure

test-quick: build
	@echo "Running quick tests..."
	cd $(BUILD_DIR) && ctest --output-on-failure -R "test_alignment|test_benchmark|test_double"

test-single:
	@echo "Usage: make test-single TEST_NAME=<test_name>"
	@echo "Example: make test-single TEST_NAME=test_alignment"
	@[ -n "$(TEST_NAME)" ] || exit 1
	./$(BUILD_DIR)/$(TEST_NAME)

lint:
	@echo "Running clang-format..."
	find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i -style=file
	@echo "Running clang-tidy..."
	@mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake .. $(CMAKE_FLAGS)
	@clang-tidy -p $(BUILD_DIR) $(shell git ls-files "*.cpp" "*.hpp") || echo "clang-tidy completed with warnings"

benchmark: build
	@echo "Running benchmark..."
	./$(BUILD_DIR)/mem_band

benchmark-quick: build
	@echo "Running quick benchmark..."
	./$(BUILD_DIR)/mem_band -s 64 -n 5

benchmark-custom: build
	@echo "Running custom benchmark..."
	@echo "Usage: make benchmark-custom SIZE=<MiB> ITERS=<N> TYPE=<float|double>"
	./$(BUILD_DIR)/mem_band --size $(SIZE) --iters $(ITERS) --type $(TYPE)

run-apu: build
	@echo "Running APU system identifier..."
	./$(BUILD_DIR)/mem_band --run-apu

run-npu: build
	@echo "Running NPU benchmark..."
	./$(BUILD_DIR)/mem_band --run-npu

run-npu-suite: build
	@echo "Running NPU benchmark suite..."
	./$(BUILD_DIR)/mem_band --run-npu-suite

run-medium-test: build
	@echo "Running medium test subset..."
	./$(BUILD_DIR)/mem_band --run-medium-test

help:
	@echo "Memory Bandwidth Benchmark - Makefile Targets"
	@echo ""
	@echo "Build targets:"
	@echo "  make build      - Build in Release mode (default)"
	@echo "  make debug      - Build in Debug mode"
	@echo "  make release    - Build in Release mode (explicit)"
	@echo "  make simd       - Build with SIMD support enabled"
	@echo "  make clean      - Remove build directory"
	@echo ""
	@echo "Test targets:"
	@echo "  make test       - Run full test suite"
	@echo "  make test-quick - Run quick tests (alignment, benchmark, double)"
	@echo "  make test-single TEST_NAME=<name> - Run specific test executable"
	@echo ""
	@echo "Benchmark targets:"
	@echo "  make benchmark        - Run default benchmark"
	@echo "  make benchmark-quick  - Run quick benchmark (64 MiB, 5 iterations)"
	@echo "  make benchmark-custom - Run custom benchmark (use SIZE=, ITERS=, TYPE=)"
	@echo ""
	@echo "Special benchmarks:"
	@echo "  make run-apu    - Run APU system identifier"
	@echo "  make run-npu    - Run NPU benchmark"
	@echo "  make run-npu-suite - Run NPU benchmark suite"
	@echo "  make run-medium-test - Run medium test subset"
	@echo ""
	@echo "Other targets:"
	@echo "  make lint       - Run clang-format and clang-tidy"
	@echo "  make help       - Show this help message"
	@echo ""
	@echo "CMake Flags:"
	@echo "  CMAKE_FLAGS=-DCMAKE_BUILD_TYPE=<type>  - Set CMake build type (default: Release)"
	@echo ""
	@echo "Examples:"
	@echo "  make build"
	@echo "  make debug"
	@echo "  make test"
	@echo "  make benchmark-custom SIZE=1024 ITERS=30 TYPE=double"
