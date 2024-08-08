# toolchain-arm64.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Where is the target environment
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu CACHE STRING "Root paths to search on the filesystem")
set(TOOLCHAIN_PREFIX aarch64-none-linux-gnu- CACHE STRING "Cross-compilation toolchain prefix")

set(TOOLCHAIN_BIN_PREFIX ${CMAKE_FIND_ROOT_PATH}/bin/${TOOLCHAIN_PREFIX})

# Specify the cross compiler
set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_PREFIX}g++)
set(CMAKE_AR ${TOOLCHAIN_BIN_PREFIX}ar)
set(CMAKE_AS ${TOOLCHAIN_BIN_PREFIX}as)
set(CMAKE_NM ${TOOLCHAIN_BIN_PREFIX}nm)
set(CMAKE_RANLIB ${TOOLCHAIN_BIN_PREFIX}ranlib)
set(CMAKE_STRIP ${TOOLCHAIN_BIN_PREFIX}strip)

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# For libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

