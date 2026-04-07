#!/usr/bin/env bash
# run_medium_tests.sh
# Runs a default/medium subset of benchmark tests (quick validation)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

echo "=== Running Medium Benchmark Test Suite ==="
echo ""
echo "Test Subset:"
echo "------------"
echo "  - test_benchmark       (Basic kernel tests)"
echo "  - test_double          (Double precision tests)"
echo "  - test_alignment       (Alignment tests)"
echo "  - test_alu             (ALU kernel tests)"
echo "  - test_ssd             (SSD I/O benchmark tests)"
echo "  - test_apu_identifier  (APU system identifier)"
echo "  - test_npu             (NPU benchmark)"
echo "  - mem_band             (Basic run with default config)"
echo "  (Excludes: long-running 1024 MiB stress test)"
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

echo "Running tests..."
echo ""

# Run medium subset with output
cd "$BUILD_DIR"
ctest --output-on-failure \
    -R "^(BenchmarkUnitTest|DoubleUnitTest|AlignmentUnitTest|ALUUnitTest|SSDUnitTest|APUIdentifierTest|NPUUnitTest|MemBandBasicRun)$"

echo ""
echo "=== Medium benchmark tests completed ==="
