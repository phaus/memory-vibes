#!/usr/bin/env bash
# run_all_tests.sh
# Runs the complete benchmark benchmark test suite

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

echo "=== Running All Benchmark Tests ==="
echo ""

# Build if needed
if [ ! -d "$BUILD_DIR" ] || [ ! -f "$BUILD_DIR/CTestTestfile.cmake" ]; then
    echo "Building project..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build . --config Release
    cd "$PROJECT_DIR"
fi

echo "Test Executables:"
echo "-----------------"
echo "  - test_benchmark       (Basic kernel tests)"
echo "  - test_double          (Double precision tests)"
echo "  - test_alignment       (Alignment tests)"
echo "  - test_alu             (ALU kernel tests)"
echo "  - test_ssd             (SSD I/O benchmark tests)"
echo "  - test_apu_identifier  (APU system identifier tests)"
echo "  - test_npu             (NPU benchmark tests)"
echo "  - mem_band             (Basic run)"
echo "  - mem_band (long run)  (1024 MiB stress test)"
echo ""

# Run the full test suite with output
cd "$BUILD_DIR"
ctest --output-on-failure --verbose

echo ""
echo "=== All benchmark tests completed ==="
