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
        # For x86/x64, use AVX2
        if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "i386|powerpc")
            target_compile_options(mem_band PRIVATE -mavx2)
        # For i386, use SSE2
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
            target_compile_options(mem_band PRIVATE -msse2)
        # For PowerPC, use AltiVec
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
            target_compile_options(mem_band PRIVATE -maltivec)
        endif()
        target_compile_definitions(mem_band PRIVATE SIMD_ENABLED)
    elseif(MSVC)
        # MSVC on x86/x64
        if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "i386|powerpc")
            target_compile_options(mem_band PRIVATE /arch:AVX2)
        # MSVC doesn't typically target i386/powerpc in modern contexts, but we guard anyway
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
            target_compile_options(mem_band PRIVATE /arch:SSE2)
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
            # MSVC doesn't support -maltivec, this would need different handling
            # For now, we'll skip SIMD for PowerPC with MSVC
            message(WARNING "SIMD not supported for PowerPC with MSVC")
        endif()
        target_compile_definitions(mem_band PRIVATE SIMD_ENABLED)
    endif()
endif()

# Enable testing
enable_testing()
add_executable(test_benchmark tests/test_benchmark.cpp)
add_executable(test_double tests/test_double.cpp)
add_executable(test_alignment tests/test_alignment.cpp)
add_executable(test_alu tests/test_alu.cpp)
add_executable(test_ssd tests/test_ssd_benchmark.cpp)
add_executable(test_apu_identifier tests/test_apu_identifier.cpp)

target_include_directories(test_benchmark PRIVATE src)
target_include_directories(test_double PRIVATE src)
target_include_directories(test_alignment PRIVATE src)
target_include_directories(test_alu PRIVATE src)
target_include_directories(test_ssd PRIVATE src)
target_include_directories(test_apu_identifier PRIVATE src)

# Link c library for sysconf on some platforms
target_link_libraries(test_apu_identifier PRIVATE c)

add_test(NAME BenchmarkUnitTest COMMAND test_benchmark)
add_test(NAME DoubleUnitTest COMMAND test_double)
add_test(NAME AlignmentUnitTest COMMAND test_alignment)
add_test(NAME ALUUnitTest COMMAND test_alu)
add_test(NAME SSDUnitTest COMMAND test_ssd)
add_test(NAME APUIdentifierTest COMMAND test_apu_identifier)
add_test(NAME MemBandBasicRun COMMAND mem_band --size 8 --iters 1 --type float)
add_test(NAME SSDSequentialWrite COMMAND test_ssd)
# Long-running test (1024 MiB) to stress cache
add_test(NAME MemBandLongRun1024 COMMAND mem_band --size 1024 --iters 2 --type float)

target_include_directories(mem_band PRIVATE src)

# Enable Release optimizations by default (legacy SIMD flags when SIMD is not explicitly enabled)
# These are kept for backward compatibility but are generally superseded by ENABLE_SIMD
if(NOT ENABLE_SIMD)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
        add_compile_options(-msse2)
    endif()

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
        add_compile_options(-maltivec)
    endif()
endif()
