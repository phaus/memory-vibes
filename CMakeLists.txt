cmake_minimum_required(VERSION 3.15)

project(mem_band LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# Optional Dependencies Configuration
# ============================================================================

# Core executable build options
option(ENABLE_SIMD "Enable SIMD vectorized kernels (AVX2/SSE2/Altivec)" OFF)
option(BUILD_TESTING "Build test executables" ON)
option(ENABLE_CUDA "Enable NVIDIA GPU benchmarking (requires CUDA toolkit)" OFF)
option(ENABLE_ROCM "Enable AMD GPU/NPU benchmarking (requires ROCm)" OFF)
option(ENABLE_JSON_OUTPUT "Enable JSON output format (requires nlohmann/json)" OFF)
option(ENABLE_SQLITE_OUTPUT "Enable SQLite persistent storage (requires SQLite3)" OFF)

# Feature detection
option(CHECK_FEATURES "Automatically detect available features" ON)

if(CHECK_FEATURES)
    # Check for CUDA
    if(ENABLE_CUDA STREQUAL "ON" OR ENABLE_CUDA STREQUAL "YES")
        find_package(CUDAToolkit QUIET)
        if(CUDAToolkit_FOUND)
            set(CUDA_AVAILABLE ON)
            message(STATUS "CUDA Toolkit found - GPU benchmarking enabled")
        else()
            message(WARNING "CUDA requested but not found - GPU benchmarking disabled")
        endif()
    endif()
    
    # Check for ROCm
    if(ENABLE_ROCM STREQUAL "ON" OR ENABLE_ROCM STREQUAL "YES")
        if(DEFINED ENV{ROCM_PATH})
            set(ROCM_PATH $ENV{ROCM_PATH})
        else()
            find_path(ROCM_PATH include/hip/hip_runtime.h
                PATHS /opt/rocm /opt/rocm/hip
            )
        endif()
        if(ROCM_PATH AND EXISTS "${ROCM_PATH}/include/hip/hip_runtime.h")
            set(ROCM_AVAILABLE ON)
            message(STATUS "ROCm found at ${ROCM_PATH} - AMD benchmarking enabled")
        else()
            message(WARNING "ROCm requested but not found - AMD benchmarking disabled")
        endif()
    endif()
    
    # Check for SQLite3
    if(ENABLE_SQLITE_OUTPUT STREQUAL "ON" OR ENABLE_SQLITE_OUTPUT STREQUAL "YES")
        find_package(SQLite3 QUIET)
        if(SQLite3_FOUND)
            set(SQLITE_AVAILABLE ON)
            message(STATUS "SQLite3 found - SQLite persistent storage enabled")
        else()
            message(WARNING "SQLite3 requested but not found - SQLite output disabled")
        endif()
    endif()
endif()

# Download and configure GoogleTest (always available for test builds)
if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    message(STATUS "GoogleTest fetched for testing")
endif()

# ============================================================================
# Core Executable (No External Dependencies)
# ============================================================================

add_library(platform_detection_lib STATIC src/platform_detection.cpp src/runtime_detection.cpp src/platform_dynamic_lib.cpp)
target_include_directories(platform_detection_lib PRIVATE src)

add_executable(mem_band src/main.cpp src/system_info.cpp src/csv_output.cpp src/json_output.cpp)
target_link_libraries(mem_band PRIVATE platform_detection_lib)

# SIMD support (optional, disabled by default)
if(ENABLE_SIMD)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
            target_compile_options(mem_band PRIVATE -msse2)
            target_compile_definitions(mem_band PRIVATE ENABLE_SSE2)
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
            target_compile_options(mem_band PRIVATE -maltivec)
            target_compile_definitions(mem_band PRIVATE ENABLE_ALTIVEC)
        else()
            target_compile_options(mem_band PRIVATE -mavx2)
            target_compile_definitions(mem_band PRIVATE ENABLE_AVX2)
        endif()
        message(STATUS "SIMD enabled with ${CMAKE_CXX_COMPILER_ID} flags")
    elseif(MSVC)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
            target_compile_options(mem_band PRIVATE /arch:SSE2)
            target_compile_definitions(mem_band PRIVATE ENABLE_SSE2)
        else()
            target_compile_options(mem_band PRIVATE /arch:AVX2)
            target_compile_definitions(mem_band PRIVATE ENABLE_AVX2)
        endif()
        message(STATUS "SIMD enabled with MSVC flags")
    endif()
endif()

target_include_directories(mem_band PRIVATE src)

# Optional SQLite output support
if(SQLITE_AVAILABLE)
    target_link_libraries(mem_band PRIVATE ${SQLite3_LIBRARIES})
    target_compile_definitions(mem_band PRIVATE ENABLE_SQLITE)
endif()

# ============================================================================
# Optional GPU Benchmark Executables
# ============================================================================

# NVIDIA GPU benchmark (CUDA)
if(CUDA_AVAILABLE OR CUDA_FOUND)
    message(STATUS "Building CUDA GPU benchmark executable")
    add_executable(mem_band_cuda src/main.cpp src/gpu_benchmark.cpp src/system_info.cpp src/csv_output.cpp)
    target_include_directories(mem_band_cuda PRIVATE src)
    if(CUDAToolkit_FOUND)
        target_link_libraries(mem_band_cuda PRIVATE CUDAToolkit::cuda_driver CUDAToolkit::cudart)
    else()
        target_link_libraries(mem_band_cuda PRIVATE cuda cudart)
    endif()
endif()

# AMD GPU/NPU benchmark (ROCm)
if(ROCM_AVAILABLE)
    message(STATUS "Building ROCm GPU/NPU benchmark executable")
    add_executable(mem_band_rocm src/main.cpp src/gpu_benchmark.cpp src/npu_benchmark.cpp src/system_info.cpp src/csv_output.cpp)
    target_include_directories(mem_band_rocm PRIVATE src ${ROCM_PATH}/include)
    target_link_libraries(mem_band_rocm PRIVATE hip rocprofiler)
endif()

# ============================================================================
# Test Executables
# ============================================================================

if(BUILD_TESTING)
    # Core unit tests
    add_executable(test_benchmark tests/test_benchmark.cpp)
    add_executable(test_double tests/test_double.cpp)
    add_executable(test_alignment tests/test_alignment.cpp)
    add_executable(test_alu tests/test_alu.cpp)
    add_executable(test_multithread tests/test_multithread.cpp)
    add_executable(test_ssd tests/test_ssd_benchmark.cpp)
    add_executable(test_apu_identifier tests/test_apu_identifier.cpp)
    add_executable(test_npu tests/test_npu.cpp)
    add_executable(test_platform_detection tests/test_platform_detection.cpp)
    add_executable(test_runtime_detection tests/test_runtime_detection.cpp)
    add_executable(test_json_output tests/test_json_output.cpp)
    add_executable(test_layout_builder tests/test_layout_builder.cpp src/layout_builder.cpp)

    target_include_directories(test_benchmark PRIVATE src)
    target_include_directories(test_double PRIVATE src)
    target_include_directories(test_alignment PRIVATE src)
    target_include_directories(test_alu PRIVATE src)
    target_include_directories(test_multithread PRIVATE src)
    target_include_directories(test_ssd PRIVATE src)
    target_include_directories(test_apu_identifier PRIVATE src)
    target_include_directories(test_npu PRIVATE src)
    # Link all tests with GoogleTest (gtest is needed for includes)
    target_link_libraries(test_benchmark GTest::gtest_main GTest::gtest)
    target_link_libraries(test_double GTest::gtest_main GTest::gtest)
    target_link_libraries(test_alignment GTest::gtest_main GTest::gtest)
    target_link_libraries(test_alu GTest::gtest_main GTest::gtest)
    target_link_libraries(test_multithread GTest::gtest_main GTest::gtest)
    target_link_libraries(test_ssd GTest::gtest_main GTest::gtest)
    target_link_libraries(test_npu GTest::gtest_main GTest::gtest)
    target_link_libraries(test_platform_detection PRIVATE platform_detection_lib GTest::gtest_main GTest::gtest)
    target_include_directories(test_platform_detection PRIVATE src)
    target_include_directories(test_runtime_detection PRIVATE src)
    target_include_directories(test_json_output PRIVATE src)
    target_include_directories(test_layout_builder PRIVATE src)
    target_link_libraries(test_layout_builder GTest::gtest_main GTest::gtest)
    add_library(json_output_lib STATIC src/json_output.cpp)
    target_link_libraries(test_runtime_detection platform_detection_lib GTest::gtest_main GTest::gtest)
    target_link_libraries(test_json_output PRIVATE json_output_lib GTest::gtest_main GTest::gtest)
    
    # APU identifier test - link gtest_main; on non-Windows also link libc
    target_link_libraries(test_apu_identifier PRIVATE GTest::gtest_main)
    if(NOT WIN32)
        target_link_libraries(test_apu_identifier PRIVATE c)
    endif()

    # Optional GPU tests (only if CUDA/ROCm available)
    if(CUDA_AVAILABLE OR CUDA_FOUND)
        add_executable(test_gpu tests/test_gpu.cpp)
        target_include_directories(test_gpu PRIVATE src)
        target_link_libraries(test_gpu GTest::gtest_main)
        if(CUDAToolkit_FOUND)
            target_link_libraries(test_gpu PRIVATE CUDAToolkit::cudart_c)
        else()
            target_link_libraries(test_gpu PRIVATE cudart)
        endif()
    endif()

    # Register test cases
    enable_testing()
    add_test(NAME BenchmarkUnitTest COMMAND test_benchmark)
    add_test(NAME DoubleUnitTest COMMAND test_double)
    add_test(NAME AlignmentUnitTest COMMAND test_alignment)
    add_test(NAME ALUUnitTest COMMAND test_alu)
    add_test(NAME MultiThreadUnitTest COMMAND test_multithread)
    add_test(NAME SSDUnitTest COMMAND test_ssd)
    add_test(NAME APUIdentifierTest COMMAND test_apu_identifier)
    add_test(NAME NPUUnitTest COMMAND test_npu)
    add_test(NAME RuntimeDetectionTest COMMAND test_runtime_detection)
    add_test(NAME PlatformDetectionTest COMMAND test_platform_detection)
    add_test(NAME JSONOutputTest COMMAND test_json_output)
    add_test(NAME LayoutBuilderTest COMMAND test_layout_builder)
add_test(NAME MemBandBasicRun COMMAND mem_band --size 8 --iters 1 --type float)
add_test(NAME MemBandQuickTest COMMAND mem_band --quick-test)
add_test(NAME SSDSequentialWrite COMMAND test_ssd)
    
    if(CUDA_AVAILABLE OR CUDA_FOUND)
        add_test(NAME GPUTest COMMAND test_gpu)
    endif()

    # Platform-specific SIMD compilation flags
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
        add_compile_options(-msse2)
    endif()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
        add_compile_options(-maltivec)
    endif()
endif()

# ============================================================================
# Installation Rules
# ============================================================================

install(TARGETS mem_band
    RUNTIME DESTINATION bin
)

if(CUDA_AVAILABLE OR CUDA_FOUND)
    install(TARGETS mem_band_cuda
        RUNTIME DESTINATION bin
    )
endif()

if(ROCM_AVAILABLE)
    install(TARGETS mem_band_rocm
        RUNTIME DESTINATION bin
    )
endif()
