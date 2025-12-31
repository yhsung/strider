/**
 * @file vector.h
 * @brief SIMD vector abstraction layer
 *
 * Provides a unified interface for SIMD operations across:
 * - x86_64: SSE2 (128-bit), AVX2 (256-bit), AVX-512 (512-bit)
 * - ARM64: NEON (128-bit), SVE (scalable)
 *
 * This header abstracts platform-specific intrinsics into a
 * common API for vectorized string operations.
 *
 * @author Strider Development Team
 * @date 2025-12-31
 */

#ifndef STRIDER_SIMD_VECTOR_H
#define STRIDER_SIMD_VECTOR_H

#include "strider/config.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Platform-specific intrinsics headers */
#if defined(STRIDER_ARCH_X86_64)
    #include <emmintrin.h>  /* SSE2 */
    #ifdef STRIDER_HAS_AVX2
        #include <immintrin.h>  /* AVX2, AVX-512 */
    #endif
#elif defined(STRIDER_ARCH_ARM64)
    #include <arm_neon.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * 128-bit Vector Type (SSE2 / NEON)
 * ======================================================================== */

/**
 * @brief 128-bit SIMD vector (16 bytes)
 *
 * Maps to:
 * - x86_64: __m128i (SSE2)
 * - ARM64: uint8x16_t (NEON)
 */
typedef struct {
    #if defined(STRIDER_ARCH_X86_64)
        __m128i data;
    #elif defined(STRIDER_ARCH_ARM64)
        uint8x16_t data;
    #else
        uint8_t data[16];  /* Scalar fallback */
    #endif
} strider_vec128_t;

/* ========================================================================
 * 256-bit Vector Type (AVX2)
 * ======================================================================== */

#if defined(STRIDER_HAS_AVX2) || !defined(STRIDER_ARCH_X86_64)
/**
 * @brief 256-bit SIMD vector (32 bytes)
 *
 * Maps to:
 * - x86_64: __m256i (AVX2)
 * - ARM64: Two NEON vectors (emulated)
 */
typedef struct {
    #if defined(STRIDER_HAS_AVX2)
        __m256i data;
    #elif defined(STRIDER_ARCH_ARM64)
        uint8x16_t data[2];  /* Two NEON registers */
    #else
        uint8_t data[32];    /* Scalar fallback */
    #endif
} strider_vec256_t;
#endif

/* ========================================================================
 * Load Operations (128-bit)
 * ======================================================================== */

/**
 * @brief Load 16 bytes from aligned memory
 *
 * @param ptr Pointer to 16-byte aligned memory
 * @return Loaded vector
 *
 * @note ptr MUST be 16-byte aligned for optimal performance
 * @note Undefined behavior if ptr is not aligned on x86 SSE
 */
static inline strider_vec128_t strider_vec128_load_aligned(const void* ptr) {
    strider_vec128_t result;
    #if defined(STRIDER_ARCH_X86_64)
        result.data = _mm_load_si128((const __m128i*)ptr);
    #elif defined(STRIDER_ARCH_ARM64)
        result.data = vld1q_u8((const uint8_t*)ptr);
    #else
        memcpy(result.data, ptr, 16);
    #endif
    return result;
}

/**
 * @brief Load 16 bytes from unaligned memory
 *
 * @param ptr Pointer to memory (any alignment)
 * @return Loaded vector
 *
 * @note Slower than aligned load on some platforms
 */
static inline strider_vec128_t strider_vec128_load_unaligned(const void* ptr) {
    strider_vec128_t result;
    #if defined(STRIDER_ARCH_X86_64)
        result.data = _mm_loadu_si128((const __m128i*)ptr);
    #elif defined(STRIDER_ARCH_ARM64)
        result.data = vld1q_u8((const uint8_t*)ptr);  /* NEON handles unaligned */
    #else
        memcpy(result.data, ptr, 16);
    #endif
    return result;
}

/* ========================================================================
 * Store Operations (128-bit)
 * ======================================================================== */

/**
 * @brief Store 16 bytes to aligned memory
 *
 * @param ptr Pointer to 16-byte aligned memory
 * @param vec Vector to store
 *
 * @note ptr MUST be 16-byte aligned
 */
static inline void strider_vec128_store_aligned(void* ptr, strider_vec128_t vec) {
    #if defined(STRIDER_ARCH_X86_64)
        _mm_store_si128((__m128i*)ptr, vec.data);
    #elif defined(STRIDER_ARCH_ARM64)
        vst1q_u8((uint8_t*)ptr, vec.data);
    #else
        memcpy(ptr, vec.data, 16);
    #endif
}

/**
 * @brief Store 16 bytes to unaligned memory
 *
 * @param ptr Pointer to memory (any alignment)
 * @param vec Vector to store
 */
static inline void strider_vec128_store_unaligned(void* ptr, strider_vec128_t vec) {
    #if defined(STRIDER_ARCH_X86_64)
        _mm_storeu_si128((__m128i*)ptr, vec.data);
    #elif defined(STRIDER_ARCH_ARM64)
        vst1q_u8((uint8_t*)ptr, vec.data);
    #else
        memcpy(ptr, vec.data, 16);
    #endif
}

/* ========================================================================
 * Set/Zero Operations (128-bit)
 * ======================================================================== */

/**
 * @brief Broadcast single byte to all lanes
 *
 * @param value Byte value to broadcast
 * @return Vector with all bytes set to value
 */
static inline strider_vec128_t strider_vec128_set1(uint8_t value) {
    strider_vec128_t result;
    #if defined(STRIDER_ARCH_X86_64)
        result.data = _mm_set1_epi8((char)value);
    #elif defined(STRIDER_ARCH_ARM64)
        result.data = vdupq_n_u8(value);
    #else
        for (int i = 0; i < 16; i++) {
            result.data[i] = value;
        }
    #endif
    return result;
}

/**
 * @brief Create zero vector
 *
 * @return Vector with all bytes set to zero
 */
static inline strider_vec128_t strider_vec128_zero(void) {
    strider_vec128_t result;
    #if defined(STRIDER_ARCH_X86_64)
        result.data = _mm_setzero_si128();
    #elif defined(STRIDER_ARCH_ARM64)
        result.data = vdupq_n_u8(0);
    #else
        for (int i = 0; i < 16; i++) {
            result.data[i] = 0;
        }
    #endif
    return result;
}

/* ========================================================================
 * 256-bit Operations (AVX2)
 * ======================================================================== */

#if defined(STRIDER_HAS_AVX2) || !defined(STRIDER_ARCH_X86_64)

static inline strider_vec256_t strider_vec256_load_aligned(const void* ptr) {
    strider_vec256_t result;
    #if defined(STRIDER_HAS_AVX2)
        result.data = _mm256_load_si256((const __m256i*)ptr);
    #elif defined(STRIDER_ARCH_ARM64)
        result.data[0] = vld1q_u8((const uint8_t*)ptr);
        result.data[1] = vld1q_u8((const uint8_t*)ptr + 16);
    #else
        memcpy(result.data, ptr, 32);
    #endif
    return result;
}

static inline void strider_vec256_store_aligned(void* ptr, strider_vec256_t vec) {
    #if defined(STRIDER_HAS_AVX2)
        _mm256_store_si256((__m256i*)ptr, vec.data);
    #elif defined(STRIDER_ARCH_ARM64)
        vst1q_u8((uint8_t*)ptr, vec.data[0]);
        vst1q_u8((uint8_t*)ptr + 16, vec.data[1]);
    #else
        memcpy(ptr, vec.data, 32);
    #endif
}

static inline strider_vec256_t strider_vec256_set1(uint8_t value) {
    strider_vec256_t result;
    #if defined(STRIDER_HAS_AVX2)
        result.data = _mm256_set1_epi8((char)value);
    #elif defined(STRIDER_ARCH_ARM64)
        result.data[0] = vdupq_n_u8(value);
        result.data[1] = vdupq_n_u8(value);
    #else
        for (int i = 0; i < 32; i++) {
            result.data[i] = value;
        }
    #endif
    return result;
}

static inline strider_vec256_t strider_vec256_zero(void) {
    strider_vec256_t result;
    #if defined(STRIDER_HAS_AVX2)
        result.data = _mm256_setzero_si256();
    #elif defined(STRIDER_ARCH_ARM64)
        result.data[0] = vdupq_n_u8(0);
        result.data[1] = vdupq_n_u8(0);
    #else
        for (int i = 0; i < 32; i++) {
            result.data[i] = 0;
        }
    #endif
    return result;
}

#endif /* AVX2 */

/* ========================================================================
 * Utility Functions
 * ======================================================================== */

/**
 * @brief Check if pointer is aligned to specified boundary
 *
 * @param ptr Pointer to check
 * @param alignment Required alignment in bytes (must be power of 2)
 * @return true if aligned, false otherwise
 */
static inline bool strider_is_aligned(const void* ptr, size_t alignment) {
    return ((uintptr_t)ptr & (alignment - 1)) == 0;
}

#ifdef __cplusplus
}
#endif

#endif /* STRIDER_SIMD_VECTOR_H */
