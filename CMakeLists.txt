cmake_minimum_required(VERSION 3.15)

project(mem_band LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Download and configure GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  DOWNLOAD_EXTRACT_TIMESTAMP OFF
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Add main executable
add_executable(mem_band src/main.cpp src/system_info.cpp src/csv_output.cpp)

# SIMD support
option(ENABLE_SIMD "Enable SIMD vectorised kernels" OFF)
if(ENABLE_SIMD)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
            target_compile_options(mem_band PRIVATE -msse2)
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
            target_compile_options(mem_band PRIVATE -maltivec)
        else()
            target_compile_options(mem_band PRIVATE -mavx2)
        endif()
        target_compile_definitions(mem_band PRIVATE SIMD_ENABLED)
    elseif(MSVC)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
            target_compile_options(mem_band PRIVATE /arch:SSE2)
        else()
            target_compile_options(mem_band PRIVATE /arch:AVX2)
        endif()
        target_compile_definitions(mem_band PRIVATE SIMD_ENABLED)
    endif()
endif()

target_include_directories(mem_band PRIVATE src)

# Enable testing
enable_testing()

add_executable(test_benchmark tests/test_benchmark.cpp)
add_executable(test_double tests/test_double.cpp)
add_executable(test_alignment tests/test_alignment.cpp)
add_executable(test_alu tests/test_alu.cpp)
add_executable(test_ssd tests/test_ssd_benchmark.cpp)
add_executable(test_apu_identifier tests/test_apu_identifier.cpp)
add_executable(test_npu tests/test_npu.cpp)

target_include_directories(test_benchmark PRIVATE src)
target_include_directories(test_double PRIVATE src)
target_include_directories(test_alignment PRIVATE src)
target_include_directories(test_alu PRIVATE src)
target_include_directories(test_ssd PRIVATE src)
target_include_directories(test_apu_identifier PRIVATE src)
target_include_directories(test_npu PRIVATE src)

target_link_libraries(test_benchmark gtest gtest_main)
target_link_libraries(test_double gtest gtest_main)
target_link_libraries(test_alignment gtest gtest_main)
target_link_libraries(test_alu gtest gtest_main)
target_link_libraries(test_ssd gtest gtest_main)
target_link_libraries(test_apu_identifier gtest gtest_main c)
target_link_libraries(test_npu gtest gtest_main)

add_test(NAME BenchmarkUnitTest COMMAND test_benchmark)
add_test(NAME DoubleUnitTest COMMAND test_double)
add_test(NAME AlignmentUnitTest COMMAND test_alignment)
add_test(NAME ALUUnitTest COMMAND test_alu)
add_test(NAME SSDUnitTest COMMAND test_ssd)
add_test(NAME APUIdentifierTest COMMAND test_apu_identifier)
add_test(NAME NPUUnitTest COMMAND test_npu)
add_test(NAME MemBandBasicRun COMMAND mem_band --size 8 --iters 1 --type float)
add_test(NAME SSDSequentialWrite COMMAND test_ssd)
add_test(NAME MemBandLongRun1024 COMMAND mem_band --size 1024 --iters 1 --type float)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
    add_compile_options(-msse2)
endif()
if(CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
    add_compile_options(-maltivec)
endif()
