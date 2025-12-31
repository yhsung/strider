/**
 * @file vector.h
 * @brief SIMD vector abstraction layer
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2025 Strider Development Team
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
 * Comparison Operations (128-bit)
 * ======================================================================== */

/**
 * @brief Element-wise equality comparison
 *
 * @param a First vector
 * @param b Second vector
 * @return Vector with 0xFF for equal bytes, 0x00 for different bytes
 *
 * @note Result can be used with movemask to extract comparison results
 */
static inline strider_vec128_t strider_vec128_cmpeq(strider_vec128_t a, strider_vec128_t b) {
    strider_vec128_t result;
    #if defined(STRIDER_ARCH_X86_64)
        result.data = _mm_cmpeq_epi8(a.data, b.data);
    #elif defined(STRIDER_ARCH_ARM64)
        result.data = vceqq_u8(a.data, b.data);
    #else
        for (int i = 0; i < 16; i++) {
            result.data[i] = (a.data[i] == b.data[i]) ? 0xFF : 0x00;
        }
    #endif
    return result;
}

/**
 * @brief Extract sign bit mask from vector bytes
 *
 * @param vec Input vector
 * @return 16-bit mask where bit i = sign bit of byte i
 *
 * @note Used with cmpeq to find matching bytes
 * @note On ARM, extracts MSB of each byte to build mask
 */
static inline uint32_t strider_vec128_movemask(strider_vec128_t vec) {
    #if defined(STRIDER_ARCH_X86_64)
        return (uint32_t)_mm_movemask_epi8(vec.data);
    #elif defined(STRIDER_ARCH_ARM64)
        /* ARM NEON: Extract MSB from each byte and pack into mask
         * Strategy: Narrow and shift to pack 16 MSBs into 16 bits */

        /* Shift each byte right by 7 to get MSB in bit 0 */
        uint8x16_t shifted = vshrq_n_u8(vec.data, 7);

        /* Now each byte is either 0x00 or 0x01 */
        /* Pack pairs of bytes into nibbles using shifts */
        uint8x16_t mask_0 = shifted;
        uint8x16_t mask_1 = vshlq_n_u8(shifted, 1);
        uint8x16_t mask_2 = vshlq_n_u8(shifted, 2);
        uint8x16_t mask_3 = vshlq_n_u8(shifted, 3);
        uint8x16_t mask_4 = vshlq_n_u8(shifted, 4);
        uint8x16_t mask_5 = vshlq_n_u8(shifted, 5);
        uint8x16_t mask_6 = vshlq_n_u8(shifted, 6);
        uint8x16_t mask_7 = vshlq_n_u8(shifted, 7);

        /* Combine adjacent pairs */
        uint8x16_t pair0 = vorrq_u8(mask_0, mask_1);
        uint8x16_t pair1 = vorrq_u8(mask_2, mask_3);
        uint8x16_t pair2 = vorrq_u8(mask_4, mask_5);
        uint8x16_t pair3 = vorrq_u8(mask_6, mask_7);

        /* Extract bytes and build mask manually */
        uint8_t bytes[16];
        vst1q_u8(bytes, shifted);

        uint32_t mask = 0;
        for (int i = 0; i < 16; i++) {
            mask |= (bytes[i] & 1) << i;
        }

        return mask;
    #else
        /* Scalar fallback */
        uint32_t mask = 0;
        for (int i = 0; i < 16; i++) {
            if (vec.data[i] & 0x80) {
                mask |= (1U << i);
            }
        }
        return mask;
    #endif
}

/* ========================================================================
 * Comparison Operations (256-bit)
 * ======================================================================== */

#if defined(STRIDER_HAS_AVX2) || !defined(STRIDER_ARCH_X86_64)

static inline strider_vec256_t strider_vec256_cmpeq(strider_vec256_t a, strider_vec256_t b) {
    strider_vec256_t result;
    #if defined(STRIDER_HAS_AVX2)
        result.data = _mm256_cmpeq_epi8(a.data, b.data);
    #elif defined(STRIDER_ARCH_ARM64)
        result.data[0] = vceqq_u8(a.data[0], b.data[0]);
        result.data[1] = vceqq_u8(a.data[1], b.data[1]);
    #else
        for (int i = 0; i < 32; i++) {
            result.data[i] = (a.data[i] == b.data[i]) ? 0xFF : 0x00;
        }
    #endif
    return result;
}

static inline uint32_t strider_vec256_movemask(strider_vec256_t vec) {
    #if defined(STRIDER_HAS_AVX2)
        return (uint32_t)_mm256_movemask_epi8(vec.data);
    #elif defined(STRIDER_ARCH_ARM64)
        /* Process two 128-bit halves separately */
        strider_vec128_t lo, hi;
        lo.data = vec.data[0];
        hi.data = vec.data[1];
        uint32_t lo_mask = strider_vec128_movemask(lo);
        uint32_t hi_mask = strider_vec128_movemask(hi);
        return lo_mask | (hi_mask << 16);
    #else
        uint32_t mask = 0;
        for (int i = 0; i < 32; i++) {
            if (vec.data[i] & 0x80) {
                mask |= (1U << i);
            }
        }
        return mask;
    #endif
}

#endif /* STRIDER_HAS_AVX2 */

/* ========================================================================
 * Bit Manipulation Utilities
 * ======================================================================== */

/**
 * @brief Count trailing zeros (find position of first set bit)
 *
 * @param x Input value
 * @return Number of trailing zero bits (0-31), or 32 if x is 0
 *
 * @note Used to find first match in movemask result
 */
static inline int strider_ctz32(uint32_t x) {
    if (x == 0) return 32;

    #if defined(__GNUC__) || defined(__clang__)
        return __builtin_ctz(x);
    #elif defined(_MSC_VER)
        unsigned long index;
        _BitScanForward(&index, x);
        return (int)index;
    #else
        /* Portable fallback using De Bruijn sequence */
        static const int debruijn32[32] = {
            0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
            31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
        };
        return debruijn32[((x & -x) * 0x077CB531U) >> 27];
    #endif
}

/**
 * @brief Count number of set bits (population count)
 *
 * @param x Input value
 * @return Number of 1 bits in x
 *
 * @note Used to count total matches in a vector
 */
static inline int strider_popcount32(uint32_t x) {
    #if defined(__GNUC__) || defined(__clang__)
        return __builtin_popcount(x);
    #elif defined(_MSC_VER) && defined(_M_X64)
        return (int)__popcnt(x);
    #else
        /* Portable fallback - Brian Kernighan's algorithm */
        int count = 0;
        while (x) {
            x &= x - 1;  /* Clear lowest set bit */
            count++;
        }
        return count;
    #endif
}

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
