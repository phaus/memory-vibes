// toolchain-i386.cmake
// Minimal CMake toolchain file for building on i386 (32‑bit x86) Linux.
// Adjust the compiler paths as needed for your environment.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR i386)

# Specify the cross‑compilers (assumes they are in PATH).
set(CMAKE_C_COMPILER   "i686-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "i686-linux-gnu-g++")

# Force 32‑bit code generation.
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
