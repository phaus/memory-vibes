cmake_minimum_required(VERSION 3.15)

project(mem_band LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(mem_band src/main.cpp)

# Enable testing
enable_testing()
add_executable(test_benchmark tests/test_benchmark.cpp)
add_executable(test_double tests/test_double.cpp)
target_include_directories(test_double PRIVATE src)
target_include_directories(test_benchmark PRIVATE src)
add_test(NAME DoubleUnitTest COMMAND test_double)
add_test(NAME MemBandBasicRun COMMAND mem_band --size 8 --iters 1 --type float)
add_test(NAME BenchmarkUnitTest COMMAND test_benchmark)

target_include_directories(mem_band PRIVATE src)

# Enable Release optimizations by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
