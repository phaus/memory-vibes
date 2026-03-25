cmake_minimum_required(VERSION 3.15)

project(mem_band LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(mem_band src/main.cpp)
# SIMD support can be enabled via the ENABLE_SIMD option.
option(ENABLE_SIMD "Enable SIMD vectorised kernels (requires appropriate SIMD support)" OFF)
if(ENABLE_SIMD)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(mem_band PRIVATE -mavx2)
        target_compile_definitions(mem_band PRIVATE SIMD_ENABLED)
    elseif(MSVC)
        target_compile_options(mem_band PRIVATE /arch:AVX2)
        target_compile_definitions(mem_band PRIVATE SIMD_ENABLED)
    endif()
endif()

# Enable testing
enable_testing()
add_executable(test_benchmark tests/test_benchmark.cpp)
add_executable(test_double tests/test_double.cpp)
add_executable(test_alignment tests/test_alignment.cpp)

target_include_directories(test_benchmark PRIVATE src)
target_include_directories(test_double PRIVATE src)
target_include_directories(test_alignment PRIVATE src)

add_test(NAME BenchmarkUnitTest COMMAND test_benchmark)
add_test(NAME DoubleUnitTest COMMAND test_double)
add_test(NAME AlignmentUnitTest COMMAND test_alignment)
add_test(NAME MemBandBasicRun COMMAND mem_band --size 8 --iters 1 --type float)
# Long-running test (1024 MiB) to stress cache
add_test(NAME MemBandLongRun1024 COMMAND mem_band --size 1024 --iters 2 --type float)

target_include_directories(mem_band PRIVATE src)

# Enable Release optimizations by default
if(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
    add_compile_options(-msse2)
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
    add_compile_options(-maltivec)
endif()
