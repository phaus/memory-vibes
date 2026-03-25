// toolchain-ppc32.cmake
// Minimal CMake toolchain file for building on PowerPC 32‑bit Linux.
// Adjust the compiler paths as needed for your environment.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR powerpc)

# Specify the cross‑compilers (assumes they are in PATH).
set(CMAKE_C_COMPILER   "powerpc-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "powerpc-linux-gnu-g++")

# Force 32‑bit code generation.
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
