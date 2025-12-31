/**
 * @file newline.h
 * @brief Newline detection and counting operations
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2025 Strider Development Team
 *
 * Provides fast newline counting for log processing, similar to `wc -l`.
 * Supports Unix (\n), Windows (\r\n), and Mac Classic (\r) formats.
 *
 * @author Strider Development Team
 * @date 2025-12-31
 */

#ifndef STRIDER_PARSERS_NEWLINE_H
#define STRIDER_PARSERS_NEWLINE_H

#include "strider/config.h"
#include "strider/utils/memory.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Count newlines in buffer (scalar reference implementation)
 *
 * Counts all newline characters: \n, \r\n (as one), and \r.
 * Behaves like `wc -l` - counts newline characters, not lines.
 *
 * @param data Buffer to scan
 * @param size Size of buffer in bytes
 * @return Number of newlines found
 *
 * @note Portable C implementation for correctness testing
 * @note Handles all newline formats (Unix, Windows, Mac Classic)
 */
size_t strider_count_newlines(const char *data, size_t size);

/**
 * @brief Count newlines in buffer (SIMD-accelerated)
 *
 * Uses SIMD instructions to count newlines in parallel.
 * Guaranteed to return same result as strider_count_newlines().
 *
 * @param data Buffer to scan
 * @param size Size of buffer in bytes
 * @return Number of newlines found
 *
 * @note Optimized for large buffers (>1KB)
 * @note Handles unaligned buffers
 * @note Automatically uses best SIMD implementation for platform
 */
size_t strider_count_newlines_simd(const char *data, size_t size);

/**
 * @brief Find positions of all newlines in buffer
 *
 * Scans buffer and records the byte offset of each newline character.
 * Useful for building line index for fast line-based access.
 *
 * @param data Buffer to scan
 * @param size Size of buffer in bytes
 * @param positions Output array to store newline positions
 * @param max_positions Maximum number of positions to store
 * @return Number of newlines found (may be > max_positions)
 *
 * @note If return value > max_positions, only first max_positions are stored
 * @note Positions point to the newline character itself
 * @note For \r\n pairs, position points to \r
 */
size_t strider_find_newline_positions(const char *data, size_t size, size_t *positions,
                                      size_t max_positions);

#ifdef __cplusplus
}
#endif

#endif /* STRIDER_PARSERS_NEWLINE_H */
