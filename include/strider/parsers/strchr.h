/**
 * @file strchr.h
 * @brief Single character search operations (strchr-like functionality)
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2025 Strider Development Team
 *
 * Provides both scalar reference and SIMD-accelerated implementations
 * of single character search (similar to standard C strchr).
 *
 * @author Strider Development Team
 * @date 2025-12-31
 */

#ifndef STRIDER_PARSERS_STRCHR_H
#define STRIDER_PARSERS_STRCHR_H

#include "strider/config.h"
#include "strider/utils/memory.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Find first occurrence of character in string (scalar reference)
 *
 * Portable C implementation that serves as reference for correctness testing.
 *
 * @param str Null-terminated string to search
 * @param ch Character to find (converted to unsigned char)
 * @return Pointer to first occurrence of ch, or NULL if not found
 *
 * @note Like standard strchr, this can find the null terminator
 * @note This is the reference implementation for testing SIMD variants
 */
const char *strider_strchr(const char *str, int ch);

/**
 * @brief Find first occurrence of character in string (SIMD-accelerated)
 *
 * Uses SIMD instructions (SSE2/AVX2/NEON) to search for character in parallel.
 * Handles unaligned buffers with prefix/suffix scalar fallback.
 *
 * @param str Null-terminated string to search
 * @param ch Character to find (converted to unsigned char)
 * @return Pointer to first occurrence of ch, or NULL if not found
 *
 * @note Guaranteed to return same result as strider_strchr()
 * @note Performance improves with longer strings (>16 bytes)
 * @note Automatically uses best SIMD implementation for platform
 */
const char *strider_strchr_simd(const char *str, int ch);

#ifdef __cplusplus
}
#endif

#endif /* STRIDER_PARSERS_STRCHR_H */
