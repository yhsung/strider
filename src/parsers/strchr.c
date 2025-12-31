/**
 * @file strchr.c
 * @brief Implementation of single character search operations
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2025 Strider Development Team
 */

#include "strider/parsers/strchr.h"
#include "strider/simd/vector.h"
#include <stdint.h>
#include <string.h>

/* ========================================================================
 * Scalar Reference Implementation
 * ======================================================================== */

const char *strider_strchr(const char *str, int ch) {
    unsigned char target = (unsigned char) ch;

    /* Search byte-by-byte until we find target or null terminator */
    while (*str != '\0') {
        if ((unsigned char) *str == target) {
            return str;
        }
        str++;
    }

    /* Check if we're searching for null terminator */
    if (target == '\0') {
        return str;
    }

    return NULL;
}

/* ========================================================================
 * SIMD Implementation Helpers
 * ======================================================================== */

/**
 * @brief Search for character in aligned 16-byte chunk using SSE2/NEON
 *
 * @param ptr Aligned pointer to 16 bytes
 * @param target_vec Vector with target byte broadcast to all lanes
 * @return Offset of first match (0-15), or -1 if not found
 */
static inline int search_chunk_128(const uint8_t *ptr, strider_vec128_t target_vec) {
    strider_vec128_t data = strider_vec128_load_aligned(ptr);
    strider_vec128_t cmp = strider_vec128_cmpeq(data, target_vec);
    uint32_t mask = strider_vec128_movemask(cmp);

    if (mask != 0) {
        return strider_ctz32(mask);
    }
    return -1;
}

#if defined(STRIDER_HAS_AVX2)
/**
 * @brief Search for character in aligned 32-byte chunk using AVX2
 *
 * @param ptr Aligned pointer to 32 bytes
 * @param target_vec Vector with target byte broadcast to all lanes
 * @return Offset of first match (0-31), or -1 if not found
 */
static inline int search_chunk_256(const uint8_t *ptr, strider_vec256_t target_vec) {
    strider_vec256_t data = strider_vec256_load_aligned(ptr);
    strider_vec256_t cmp = strider_vec256_cmpeq(data, target_vec);
    uint32_t mask = strider_vec256_movemask(cmp);

    if (mask != 0) {
        return strider_ctz32(mask);
    }
    return -1;
}
#endif

/**
 * @brief Check if null terminator exists in vector
 *
 * @param ptr Pointer to data
 * @param len Length to check
 * @return Offset of null terminator, or -1 if not found
 */
static inline int find_null_terminator_128(const uint8_t *ptr) {
    strider_vec128_t data = strider_vec128_load_aligned(ptr);
    strider_vec128_t zero = strider_vec128_zero();
    strider_vec128_t cmp = strider_vec128_cmpeq(data, zero);
    uint32_t mask = strider_vec128_movemask(cmp);

    if (mask != 0) {
        return strider_ctz32(mask);
    }
    return -1;
}

#if defined(STRIDER_HAS_AVX2)
static inline int find_null_terminator_256(const uint8_t *ptr) {
    strider_vec256_t data = strider_vec256_load_aligned(ptr);
    strider_vec256_t zero = strider_vec256_zero();
    strider_vec256_t cmp = strider_vec256_cmpeq(data, zero);
    uint32_t mask = strider_vec256_movemask(cmp);

    if (mask != 0) {
        return strider_ctz32(mask);
    }
    return -1;
}
#endif

/* ========================================================================
 * SIMD Implementation
 * ======================================================================== */

const char *strider_strchr_simd(const char *str, int ch) {
    const uint8_t *ptr = (const uint8_t *) str;
    unsigned char target = (unsigned char) ch;

#if defined(STRIDER_HAS_AVX2)
    const size_t VECTOR_SIZE = 32;
#else
    const size_t VECTOR_SIZE = 16;
#endif

    /* Handle unaligned prefix with scalar search */
    uintptr_t addr = (uintptr_t) ptr;
    size_t prefix_len = (VECTOR_SIZE - (addr & (VECTOR_SIZE - 1))) & (VECTOR_SIZE - 1);

    if (prefix_len > 0) {
        for (size_t i = 0; i < prefix_len; i++) {
            if (ptr[i] == target) {
                return (const char *) (ptr + i);
            }
            if (ptr[i] == '\0') {
                /* Reached end, check if searching for null */
                return (target == '\0') ? (const char *) (ptr + i) : NULL;
            }
        }
        ptr += prefix_len;
    }

    /* Now ptr is aligned, use SIMD for main search */
#if defined(STRIDER_HAS_AVX2)
    strider_vec256_t target_vec = strider_vec256_set1(target);
    strider_vec256_t zero_vec = strider_vec256_zero();

    while (1) {
        /* Check for null terminator first */
        strider_vec256_t data = strider_vec256_load_aligned(ptr);
        strider_vec256_t null_cmp = strider_vec256_cmpeq(data, zero_vec);
        uint32_t null_mask = strider_vec256_movemask(null_cmp);

        /* Check for target character */
        strider_vec256_t target_cmp = strider_vec256_cmpeq(data, target_vec);
        uint32_t target_mask = strider_vec256_movemask(target_cmp);

        if (null_mask != 0) {
            int null_pos = strider_ctz32(null_mask);
            if (target_mask != 0) {
                int target_pos = strider_ctz32(target_mask);
                /* Return whichever comes first */
                if (target_pos < null_pos) {
                    return (const char *) (ptr + target_pos);
                }
            }
            /* Reached end without finding target */
            return (target == '\0') ? (const char *) (ptr + null_pos) : NULL;
        }

        if (target_mask != 0) {
            int pos = strider_ctz32(target_mask);
            return (const char *) (ptr + pos);
        }

        ptr += 32;
    }
#else
    /* SSE2/NEON path */
    strider_vec128_t target_vec = strider_vec128_set1(target);
    strider_vec128_t zero_vec = strider_vec128_zero();

    while (1) {
        /* Check for null terminator first */
        strider_vec128_t data = strider_vec128_load_aligned(ptr);
        strider_vec128_t null_cmp = strider_vec128_cmpeq(data, zero_vec);
        uint32_t null_mask = strider_vec128_movemask(null_cmp);

        /* Check for target character */
        strider_vec128_t target_cmp = strider_vec128_cmpeq(data, target_vec);
        uint32_t target_mask = strider_vec128_movemask(target_cmp);

        if (null_mask != 0) {
            int null_pos = strider_ctz32(null_mask);
            if (target_mask != 0) {
                int target_pos = strider_ctz32(target_mask);
                /* Return whichever comes first */
                if (target_pos < null_pos) {
                    return (const char *) (ptr + target_pos);
                }
            }
            /* Reached end without finding target */
            return (target == '\0') ? (const char *) (ptr + null_pos) : NULL;
        }

        if (target_mask != 0) {
            int pos = strider_ctz32(target_mask);
            return (const char *) (ptr + pos);
        }

        ptr += 16;
    }
#endif
}
