/**
 * @file test_memory_utils.c
 * @brief Unit tests for memory utility functions
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2025 Strider Development Team
 *
 * TDD Cycle 5: Memory Utilities
 * Tests aligned allocation, deallocation, and buffer view helpers.
 */

#include "unity.h"
#include "strider/utils/memory.h"
#include <string.h>

void setUp(void) {
    /* Setup before each test */
}

void tearDown(void) {
    /* Cleanup after each test */
}

/* ========================================================================
 * Aligned Allocation Tests
 * ======================================================================== */

/**
 * Test: Allocate 16-byte aligned memory
 */
void test_aligned_alloc_16(void) {
    size_t size = 64;
    void* ptr = strider_aligned_alloc(16, size);

    TEST_ASSERT_NOT_NULL(ptr);
    TEST_ASSERT_TRUE(strider_is_aligned(ptr, 16));

    /* Should also be aligned to smaller powers of 2 */
    TEST_ASSERT_TRUE(strider_is_aligned(ptr, 8));
    TEST_ASSERT_TRUE(strider_is_aligned(ptr, 4));

    strider_aligned_free(ptr);
}

/**
 * Test: Allocate 32-byte aligned memory (for AVX2)
 */
void test_aligned_alloc_32(void) {
    size_t size = 128;
    void* ptr = strider_aligned_alloc(32, size);

    TEST_ASSERT_NOT_NULL(ptr);
    TEST_ASSERT_TRUE(strider_is_aligned(ptr, 32));
    TEST_ASSERT_TRUE(strider_is_aligned(ptr, 16));

    strider_aligned_free(ptr);
}

/**
 * Test: Allocate 64-byte aligned memory (for cache line)
 */
void test_aligned_alloc_64(void) {
    size_t size = 256;
    void* ptr = strider_aligned_alloc(64, size);

    TEST_ASSERT_NOT_NULL(ptr);
    TEST_ASSERT_TRUE(strider_is_aligned(ptr, 64));
    TEST_ASSERT_TRUE(strider_is_aligned(ptr, 32));
    TEST_ASSERT_TRUE(strider_is_aligned(ptr, 16));

    strider_aligned_free(ptr);
}

/**
 * Test: Free aligned memory (should not crash)
 */
void test_aligned_free(void) {
    void* ptr1 = strider_aligned_alloc(16, 64);
    void* ptr2 = strider_aligned_alloc(32, 128);
    void* ptr3 = strider_aligned_alloc(64, 256);

    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
    TEST_ASSERT_NOT_NULL(ptr3);

    /* Should not crash */
    strider_aligned_free(ptr1);
    strider_aligned_free(ptr2);
    strider_aligned_free(ptr3);

    /* Freeing NULL should be safe */
    strider_aligned_free(NULL);

    TEST_PASS();
}

/**
 * Test: Write and read from aligned memory
 */
void test_aligned_memory_readwrite(void) {
    size_t size = 64;
    uint8_t* ptr = (uint8_t*)strider_aligned_alloc(32, size);

    TEST_ASSERT_NOT_NULL(ptr);

    /* Write pattern */
    for (size_t i = 0; i < size; i++) {
        ptr[i] = (uint8_t)i;
    }

    /* Verify pattern */
    for (size_t i = 0; i < size; i++) {
        TEST_ASSERT_EQUAL_UINT8((uint8_t)i, ptr[i]);
    }

    strider_aligned_free(ptr);
}

/* ========================================================================
 * Alignment Check Tests
 * ======================================================================== */

/**
 * Test: Check alignment validation
 */
void test_is_aligned(void) {
    /* Stack variables have platform-specific alignment */
    #if defined(_MSC_VER)
        __declspec(align(64)) uint8_t buffer[64];
    #else
        uint8_t buffer[64] __attribute__((aligned(64)));
    #endif

    TEST_ASSERT_TRUE(strider_is_aligned(buffer, 1));
    TEST_ASSERT_TRUE(strider_is_aligned(buffer, 2));
    TEST_ASSERT_TRUE(strider_is_aligned(buffer, 4));
    TEST_ASSERT_TRUE(strider_is_aligned(buffer, 8));
    TEST_ASSERT_TRUE(strider_is_aligned(buffer, 16));
    TEST_ASSERT_TRUE(strider_is_aligned(buffer, 32));
    TEST_ASSERT_TRUE(strider_is_aligned(buffer, 64));

    /* Misaligned pointers */
    TEST_ASSERT_FALSE(strider_is_aligned(buffer + 1, 2));
    TEST_ASSERT_FALSE(strider_is_aligned(buffer + 1, 4));
    TEST_ASSERT_FALSE(strider_is_aligned(buffer + 2, 4));
    TEST_ASSERT_FALSE(strider_is_aligned(buffer + 7, 8));
}

/* ========================================================================
 * Buffer View Tests
 * ======================================================================== */

/**
 * Test: Create buffer view from memory
 */
void test_buffer_view_create(void) {
    const char* test_str = "Hello, SIMD world!";
    size_t len = strlen(test_str);

    strider_buffer_view_t view = strider_buffer_view_create(test_str, len);

    TEST_ASSERT_EQUAL_PTR(test_str, view.data);
    TEST_ASSERT_EQUAL_size_t(len, view.size);
}

/**
 * Test: Create buffer view from C string
 */
void test_buffer_view_from_cstr(void) {
    const char* test_str = "Test string";

    strider_buffer_view_t view = strider_buffer_view_from_cstr(test_str);

    TEST_ASSERT_EQUAL_PTR(test_str, view.data);
    TEST_ASSERT_EQUAL_size_t(strlen(test_str), view.size);
}

/**
 * Test: Empty buffer view
 */
void test_buffer_view_empty(void) {
    strider_buffer_view_t view = strider_buffer_view_create(NULL, 0);

    TEST_ASSERT_NULL(view.data);
    TEST_ASSERT_EQUAL_size_t(0, view.size);
    TEST_ASSERT_TRUE(strider_buffer_view_is_empty(view));
}

/**
 * Test: Non-empty buffer view
 */
void test_buffer_view_not_empty(void) {
    const char* test_str = "X";
    strider_buffer_view_t view = strider_buffer_view_from_cstr(test_str);

    TEST_ASSERT_FALSE(strider_buffer_view_is_empty(view));
}

/**
 * Test: Buffer view slicing
 */
void test_buffer_view_slice(void) {
    const char* test_str = "0123456789";
    strider_buffer_view_t view = strider_buffer_view_from_cstr(test_str);

    /* Slice middle portion: "23456" */
    strider_buffer_view_t slice = strider_buffer_view_slice(view, 2, 5);

    TEST_ASSERT_EQUAL_PTR(test_str + 2, slice.data);
    TEST_ASSERT_EQUAL_size_t(5, slice.size);

    /* Verify content */
    TEST_ASSERT_EQUAL_CHAR('2', slice.data[0]);
    TEST_ASSERT_EQUAL_CHAR('6', slice.data[4]);
}

/**
 * Test: Buffer view slicing at boundaries
 */
void test_buffer_view_slice_boundaries(void) {
    const char* test_str = "ABCDEF";
    strider_buffer_view_t view = strider_buffer_view_from_cstr(test_str);

    /* Slice from start */
    strider_buffer_view_t start = strider_buffer_view_slice(view, 0, 3);
    TEST_ASSERT_EQUAL_PTR(test_str, start.data);
    TEST_ASSERT_EQUAL_size_t(3, start.size);

    /* Slice to end */
    strider_buffer_view_t end = strider_buffer_view_slice(view, 3, 3);
    TEST_ASSERT_EQUAL_PTR(test_str + 3, end.data);
    TEST_ASSERT_EQUAL_size_t(3, end.size);
}

/* ========================================================================
 * Test Runner
 * ======================================================================== */

int main(void) {
    UNITY_BEGIN();

    /* Aligned allocation tests */
    RUN_TEST(test_aligned_alloc_16);
    RUN_TEST(test_aligned_alloc_32);
    RUN_TEST(test_aligned_alloc_64);
    RUN_TEST(test_aligned_free);
    RUN_TEST(test_aligned_memory_readwrite);

    /* Alignment check tests */
    RUN_TEST(test_is_aligned);

    /* Buffer view tests */
    RUN_TEST(test_buffer_view_create);
    RUN_TEST(test_buffer_view_from_cstr);
    RUN_TEST(test_buffer_view_empty);
    RUN_TEST(test_buffer_view_not_empty);
    RUN_TEST(test_buffer_view_slice);
    RUN_TEST(test_buffer_view_slice_boundaries);

    return UNITY_END();
}
