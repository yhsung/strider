cmake_minimum_required(VERSION 3.15)
project(strider VERSION 0.1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build options
option(STRIDER_BUILD_TESTS "Build tests" ON)
option(STRIDER_BUILD_EXAMPLES "Build examples" ON)
option(STRIDER_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(STRIDER_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

# Detect platform and SIMD capabilities
include(CheckCSourceCompiles)
include(CheckIncludeFile)

# Platform detection
if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64)|(AMD64)|(amd64)")
    set(STRIDER_ARCH_X86_64 ON)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "(aarch64)|(arm64)")
    set(STRIDER_ARCH_ARM64 ON)
endif()

# Compiler flags for SIMD
if(STRIDER_ARCH_X86_64)
    if(MSVC)
        set(STRIDER_SIMD_FLAGS "/arch:AVX2")
    else()
        set(STRIDER_SIMD_FLAGS "-mavx2" "-mfma")
    endif()
elseif(STRIDER_ARCH_ARM64)
    if(NOT MSVC)
        set(STRIDER_SIMD_FLAGS "")  # NEON is enabled by default on ARM64
    endif()
endif()

# Library (header-only interface + runtime detection implementation)
add_library(strider
    src/config.c
    src/parsers/strchr.c
)
target_include_directories(strider PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Apply SIMD flags to targets that use strider
target_compile_options(strider PUBLIC ${STRIDER_SIMD_FLAGS})

# Enable testing
if(STRIDER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Build examples
if(STRIDER_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation
install(DIRECTORY include/strider DESTINATION include)
