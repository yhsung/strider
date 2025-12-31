/**
 * @file memory.h
 * @brief Memory utilities for aligned allocation and buffer management
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2025 Strider Development Team
 *
 * Provides utilities for:
 * - Aligned memory allocation (for SIMD operations)
 * - Zero-copy buffer views
 * - Alignment checking
 *
 * @author Strider Development Team
 * @date 2025-12-31
 */

#ifndef STRIDER_UTILS_MEMORY_H
#define STRIDER_UTILS_MEMORY_H

#include "strider/config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Aligned Memory Allocation
 * ======================================================================== */

/**
 * @brief Allocate aligned memory
 *
 * @param alignment Required alignment in bytes (must be power of 2)
 * @param size Size of allocation in bytes
 * @return Pointer to aligned memory, or NULL on failure
 *
 * @note Use strider_aligned_free() to deallocate
 * @note Alignment must be power of 2 and >= sizeof(void*)
 */
static inline void *strider_aligned_alloc(size_t alignment, size_t size) {
#if defined(_MSC_VER)
    return _aligned_malloc(size, alignment);
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    /* C11 aligned_alloc */
    /* Note: C11 requires size to be multiple of alignment */
    size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);
    return aligned_alloc(alignment, aligned_size);
#else
    /* POSIX posix_memalign */
    void *ptr = NULL;
    if (posix_memalign(&ptr, alignment, size) == 0) {
        return ptr;
    }
    return NULL;
#endif
}

/**
 * @brief Free aligned memory
 *
 * @param ptr Pointer returned by strider_aligned_alloc()
 *
 * @note Safe to call with NULL pointer
 */
static inline void strider_aligned_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

#if defined(_MSC_VER)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

/**
 * @brief Check if pointer is aligned to specified boundary
 *
 * @param ptr Pointer to check
 * @param alignment Required alignment in bytes (must be power of 2)
 * @return true if aligned, false otherwise
 */
static inline bool strider_is_aligned(const void *ptr, size_t alignment) {
    return ((uintptr_t) ptr & (alignment - 1)) == 0;
}

/* ========================================================================
 * Buffer View (Zero-Copy String/Data View)
 * ======================================================================== */

/**
 * @brief Non-owning view into a memory buffer
 *
 * Similar to std::string_view in C++, this provides zero-copy
 * access to existing memory without ownership.
 */
typedef struct {
    const uint8_t *data; /**< Pointer to buffer start */
    size_t size;         /**< Size of buffer in bytes */
} strider_buffer_view_t;

/**
 * @brief Create a buffer view from memory
 *
 * @param data Pointer to buffer (not owned)
 * @param size Size in bytes
 * @return Buffer view
 *
 * @note Does not copy or take ownership of data
 */
static inline strider_buffer_view_t strider_buffer_view_create(const void *data, size_t size) {
    strider_buffer_view_t view;
    view.data = (const uint8_t *) data;
    view.size = size;
    return view;
}

/**
 * @brief Create a buffer view from a C string
 *
 * @param str Null-terminated C string (not owned)
 * @return Buffer view (does not include null terminator)
 *
 * @note Uses strlen(), so str must be null-terminated
 */
static inline strider_buffer_view_t strider_buffer_view_from_cstr(const char *str) {
    return strider_buffer_view_create(str, strlen(str));
}

/**
 * @brief Check if buffer view is empty
 *
 * @param view Buffer view to check
 * @return true if size is 0, false otherwise
 */
static inline bool strider_buffer_view_is_empty(strider_buffer_view_t view) {
    return view.size == 0;
}

/**
 * @brief Create a slice (sub-view) of a buffer
 *
 * @param view Original view
 * @param offset Starting offset in bytes
 * @param length Length of slice in bytes
 * @return New view pointing to slice
 *
 * @note Does not validate bounds - caller must ensure offset + length <= view.size
 */
static inline strider_buffer_view_t strider_buffer_view_slice(strider_buffer_view_t view,
                                                              size_t offset, size_t length) {
    strider_buffer_view_t slice;
    slice.data = view.data + offset;
    slice.size = length;
    return slice;
}

/**
 * @brief Compare two buffer views for equality
 *
 * @param a First view
 * @param b Second view
 * @return true if contents are identical, false otherwise
 */
static inline bool strider_buffer_view_equals(strider_buffer_view_t a, strider_buffer_view_t b) {
    if (a.size != b.size) {
        return false;
    }
    return memcmp(a.data, b.data, a.size) == 0;
}

#ifdef __cplusplus
}
#endif

#endif /* STRIDER_UTILS_MEMORY_H */
