/**
 * @file newline.c
 * @brief Implementation of newline detection and counting
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2025 Strider Development Team
 */

#include "strider/parsers/newline.h"
#include "strider/simd/vector.h"
#include <stdint.h>

/* ========================================================================
 * Scalar Reference Implementation
 * ======================================================================== */

size_t strider_count_newlines(const char *data, size_t size) {
    const uint8_t *ptr = (const uint8_t *) data;
    size_t count = 0;

    for (size_t i = 0; i < size; i++) {
        if (ptr[i] == '\n') {
            count++;
        } else if (ptr[i] == '\r') {
            /* Check if next char is \n (Windows-style \r\n) */
            if (i + 1 < size && ptr[i + 1] == '\n') {
                i++; /* Skip the \n, count \r\n as one */
            }
            count++;
        }
    }

    return count;
}

size_t strider_find_newline_positions(const char *data, size_t size, size_t *positions,
                                      size_t max_positions) {
    const uint8_t *ptr = (const uint8_t *) data;
    size_t count = 0;

    for (size_t i = 0; i < size; i++) {
        if (ptr[i] == '\n') {
            if (count < max_positions) {
                positions[count] = i;
            }
            count++;
        } else if (ptr[i] == '\r') {
            if (count < max_positions) {
                positions[count] = i;
            }
            count++;

            /* Check if next char is \n (Windows-style \r\n) */
            if (i + 1 < size && ptr[i + 1] == '\n') {
                i++; /* Skip the \n */
            }
        }
    }

    return count;
}

/* ========================================================================
 * SIMD Implementation
 * ======================================================================== */

size_t strider_count_newlines_simd(const char *data, size_t size) {
    const uint8_t *ptr = (const uint8_t *) data;
    size_t count = 0;

#if defined(STRIDER_HAS_AVX2)
    const size_t VECTOR_SIZE = 32;
#else
    const size_t VECTOR_SIZE = 16;
#endif

    /* Handle unaligned prefix with scalar */
    uintptr_t addr = (uintptr_t) ptr;
    size_t prefix_len = (VECTOR_SIZE - (addr & (VECTOR_SIZE - 1))) & (VECTOR_SIZE - 1);

    if (prefix_len > 0 && prefix_len < size) {
        count += strider_count_newlines((const char *) ptr, prefix_len);
        ptr += prefix_len;
        size -= prefix_len;
    }

    /* Process aligned chunks with SIMD */
#if defined(STRIDER_HAS_AVX2)
    strider_vec256_t lf_vec = strider_vec256_set1('\n');
    strider_vec256_t cr_vec = strider_vec256_set1('\r');

    while (size >= 32) {
        strider_vec256_t data_vec = strider_vec256_load_aligned(ptr);

        /* Find all \n characters */
        strider_vec256_t lf_cmp = strider_vec256_cmpeq(data_vec, lf_vec);
        uint32_t lf_mask = strider_vec256_movemask(lf_cmp);

        /* Find all \r characters */
        strider_vec256_t cr_cmp = strider_vec256_cmpeq(data_vec, cr_vec);
        uint32_t cr_mask = strider_vec256_movemask(cr_cmp);

        /* Count \n occurrences */
        count += strider_popcount32(lf_mask);

        /* For \r, need to check if followed by \n */
        if (cr_mask != 0) {
            /* Simple approach: check each \r */
            for (int i = 0; i < 32; i++) {
                if (cr_mask & (1U << i)) {
                    /* Check if this \r is followed by \n */
                    if (i + 1 < 32) {
                        /* Next char is in same vector */
                        if (lf_mask & (1U << (i + 1))) {
                            /* \r\n pair - already counted the \n, skip this \r */
                            continue;
                        }
                    } else {
                        /* \r is at end of vector, check next byte */
                        if (size > 32) {
                            if (ptr[32] == '\n') {
                                continue; /* Will be counted when we process next vector */
                            }
                        }
                    }
                    count++;
                }
            }
        }

        ptr += 32;
        size -= 32;
    }
#else
    /* SSE2/NEON path */
    strider_vec128_t lf_vec = strider_vec128_set1('\n');
    strider_vec128_t cr_vec = strider_vec128_set1('\r');

    while (size >= 16) {
        strider_vec128_t data_vec = strider_vec128_load_aligned(ptr);

        /* Find all \n characters */
        strider_vec128_t lf_cmp = strider_vec128_cmpeq(data_vec, lf_vec);
        uint32_t lf_mask = strider_vec128_movemask(lf_cmp);

        /* Find all \r characters */
        strider_vec128_t cr_cmp = strider_vec128_cmpeq(data_vec, cr_vec);
        uint32_t cr_mask = strider_vec128_movemask(cr_cmp);

        /* Count \n occurrences */
        count += strider_popcount32(lf_mask);

        /* For \r, need to check if followed by \n */
        if (cr_mask != 0) {
            for (int i = 0; i < 16; i++) {
                if (cr_mask & (1U << i)) {
                    /* Check if this \r is followed by \n */
                    if (i + 1 < 16) {
                        if (lf_mask & (1U << (i + 1))) {
                            continue; /* \r\n pair */
                        }
                    } else {
                        if (size > 16) {
                            if (ptr[16] == '\n') {
                                continue;
                            }
                        }
                    }
                    count++;
                }
            }
        }

        ptr += 16;
        size -= 16;
    }
#endif

    /* Handle remaining bytes with scalar */
    if (size > 0) {
        count += strider_count_newlines((const char *) ptr, size);
    }

    return count;
}
