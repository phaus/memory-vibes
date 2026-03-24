cmake_minimum_required(VERSION 3.15)

project(mem_band LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(mem_band src/main.cpp)

target_include_directories(mem_band PRIVATE src)

# Enable Release optimizations by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
