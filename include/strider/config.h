/**
 * @file config.h
 * @brief Strider configuration and CPU feature detection
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2025 Strider Development Team
 *
 * Provides compile-time and runtime detection of SIMD capabilities
 * across x86_64 (SSE2/AVX2/AVX-512) and ARM64 (NEON) architectures.
 *
 * @author Strider Development Team
 * @date 2025-12-31
 */

#ifndef STRIDER_CONFIG_H
#define STRIDER_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Compile-Time Platform Detection
 * ======================================================================== */

#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64)
    #define STRIDER_ARCH_X86_64 1
    #define STRIDER_ARCH_NAME "x86_64"
#elif defined(__aarch64__) || defined(__arm64__) || defined(_M_ARM64)
    #define STRIDER_ARCH_ARM64 1
    #define STRIDER_ARCH_NAME "ARM64"
#else
    #define STRIDER_ARCH_UNKNOWN 1
    #define STRIDER_ARCH_NAME "Unknown"
    #warning "Strider: Unknown architecture - SIMD optimizations disabled"
#endif

/* ========================================================================
 * Compile-Time SIMD Feature Detection
 * ======================================================================== */

/* x86_64 SIMD features */
#if defined(STRIDER_ARCH_X86_64)
    /* SSE2 is baseline for x86_64 */
    #if defined(__SSE2__) || defined(_M_X64)
        #define STRIDER_HAS_SSE2 1
    #endif

    /* AVX2 */
    #if defined(__AVX2__)
        #define STRIDER_HAS_AVX2 1
    #endif

    /* AVX-512 Foundation */
    #if defined(__AVX512F__)
        #define STRIDER_HAS_AVX512F 1
    #endif

    /* AVX-512 Byte and Word */
    #if defined(__AVX512BW__)
        #define STRIDER_HAS_AVX512BW 1
    #endif
#endif

/* ARM64 SIMD features */
#if defined(STRIDER_ARCH_ARM64)
    /* NEON is standard on ARM64 */
    #if defined(__ARM_NEON) || defined(__ARM_NEON__)
        #define STRIDER_HAS_NEON 1
    #endif

    /* SVE (Scalable Vector Extension) - future */
    #if defined(__ARM_FEATURE_SVE)
        #define STRIDER_HAS_SVE 1
    #endif
#endif

/* ========================================================================
 * Runtime CPU Feature Detection
 * ======================================================================== */

/**
 * @brief CPU feature flags structure
 *
 * Contains boolean flags for all supported SIMD features.
 * Populated at runtime via CPUID (x86) or hwcaps (ARM).
 */
typedef struct {
    /* Architecture flags */
    bool arch_x86_64;
    bool arch_arm64;

    /* x86_64 features */
    bool has_sse2;
    bool has_sse3;
    bool has_ssse3;
    bool has_sse4_1;
    bool has_sse4_2;
    bool has_avx;
    bool has_avx2;
    bool has_avx512f;   /* AVX-512 Foundation */
    bool has_avx512bw;  /* AVX-512 Byte and Word */

    /* ARM64 features */
    bool has_neon;
    bool has_sve;       /* Scalable Vector Extension */
    bool has_sve2;      /* SVE2 */

    /* CPU info */
    char vendor[13];    /* CPU vendor string (x86) */
    uint32_t family;
    uint32_t model;
} strider_cpu_features_t;

/**
 * @brief Get CPU features (runtime detection)
 *
 * Queries the CPU for supported SIMD instructions using:
 * - CPUID instruction on x86_64
 * - getauxval(AT_HWCAP) on ARM64 Linux
 * - sysctlbyname on ARM64 macOS
 *
 * Results are cached after first call for performance.
 *
 * @return strider_cpu_features_t Structure with detected features
 *
 * @note Thread-safe after initialization
 * @note Results are consistent across calls
 */
strider_cpu_features_t strider_get_cpu_features(void);

/**
 * @brief Get a human-readable description of detected features
 *
 * @param features Pointer to features structure
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return Number of characters written (excluding null terminator)
 */
int strider_describe_cpu_features(const strider_cpu_features_t* features,
                                    char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* STRIDER_CONFIG_H */
