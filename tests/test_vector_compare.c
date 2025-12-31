/**
 * @file test_vector_compare.c
 * @brief Unit tests for SIMD vector comparison operations
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2025 Strider Development Team
 *
 * TDD Cycle 4: Vector Comparison Operations
 * Tests equality comparisons, movemask extraction, and byte searching.
 */

#include "unity.h"
#include "strider/simd/vector.h"
#include <string.h>

/* Test data buffers */
#if defined(_MSC_VER)
    #define ALIGN_32 __declspec(align(32))
#else
    #define ALIGN_32 __attribute__((aligned(32)))
#endif

static ALIGN_32 uint8_t test_data_a[32];
static ALIGN_32 uint8_t test_data_b[32];

void setUp(void) {
    /* Initialize test data */
    for (int i = 0; i < 32; i++) {
        test_data_a[i] = i;
        test_data_b[i] = i;
    }
}

void tearDown(void) {
    /* Cleanup if needed */
}

/* ========================================================================
 * 128-bit Comparison Tests
 * ======================================================================== */

/**
 * Test: Compare equal vectors produces all-ones mask
 */
void test_vec128_cmpeq_equal_vectors(void) {
    strider_vec128_t a = strider_vec128_load_aligned(test_data_a);
    strider_vec128_t b = strider_vec128_load_aligned(test_data_b);

    strider_vec128_t result = strider_vec128_cmpeq(a, b);

    /* Extract mask - all bits should be set (0xFFFF for 16 bytes) */
    uint32_t mask = strider_vec128_movemask(result);
    TEST_ASSERT_EQUAL_UINT32(0xFFFF, mask);
}

/**
 * Test: Compare different vectors produces correct mask
 */
void test_vec128_cmpeq_different_vectors(void) {
    /* Make byte 5 and byte 10 different */
    test_data_b[5] = 0xFF;
    test_data_b[10] = 0xFF;

    strider_vec128_t a = strider_vec128_load_aligned(test_data_a);
    strider_vec128_t b = strider_vec128_load_aligned(test_data_b);

    strider_vec128_t result = strider_vec128_cmpeq(a, b);
    uint32_t mask = strider_vec128_movemask(result);

    /* Bits 5 and 10 should be clear, all others set */
    uint32_t expected = 0xFFFF & ~(1 << 5) & ~(1 << 10);
    TEST_ASSERT_EQUAL_UINT32(expected, mask);
}

/**
 * Test: Find specific byte in vector (like strchr)
 */
void test_vec128_find_byte(void) {
    /* Set up data with target byte at position 7 */
    memset(test_data_a, 0, 16);
    test_data_a[7] = 'X';

    strider_vec128_t haystack = strider_vec128_load_aligned(test_data_a);
    strider_vec128_t needle = strider_vec128_set1('X');

    strider_vec128_t result = strider_vec128_cmpeq(haystack, needle);
    uint32_t mask = strider_vec128_movemask(result);

    /* Only bit 7 should be set */
    TEST_ASSERT_EQUAL_UINT32(1 << 7, mask);
}

/**
 * Test: Find newline character (common log parsing operation)
 */
void test_vec128_find_newline(void) {
    /* Simulate log line with newline at end */
    memcpy(test_data_a, "Log entry here\n", 15);

    strider_vec128_t haystack = strider_vec128_load_aligned(test_data_a);
    strider_vec128_t needle = strider_vec128_set1('\n');

    strider_vec128_t result = strider_vec128_cmpeq(haystack, needle);
    uint32_t mask = strider_vec128_movemask(result);

    /* Bit 14 should be set (newline at index 14) */
    TEST_ASSERT_EQUAL_UINT32(1 << 14, mask);
}

/**
 * Test: Movemask with all zeros
 */
void test_vec128_movemask_all_zeros(void) {
    strider_vec128_t vec = strider_vec128_zero();
    uint32_t mask = strider_vec128_movemask(vec);

    /* All bytes are 0x00, so no sign bits set */
    TEST_ASSERT_EQUAL_UINT32(0, mask);
}

/**
 * Test: Movemask with alternating pattern
 */
void test_vec128_movemask_pattern(void) {
    /* Create pattern: 0xFF at even indices, 0x00 at odd */
    for (int i = 0; i < 16; i++) {
        test_data_a[i] = (i % 2 == 0) ? 0xFF : 0x00;
    }

    strider_vec128_t vec = strider_vec128_load_aligned(test_data_a);
    uint32_t mask = strider_vec128_movemask(vec);

    /* Bits should be set for even positions (0, 2, 4, ..., 14) */
    uint32_t expected = 0x5555;  /* 0101010101010101 in binary */
    TEST_ASSERT_EQUAL_UINT32(expected, mask);
}

/* ========================================================================
 * 256-bit Comparison Tests (AVX2 only)
 * ======================================================================== */

#if defined(STRIDER_HAS_AVX2)

/**
 * Test: Compare equal 256-bit vectors
 */
void test_vec256_cmpeq_equal_vectors(void) {
    strider_vec256_t a = strider_vec256_load_aligned(test_data_a);
    strider_vec256_t b = strider_vec256_load_aligned(test_data_b);

    strider_vec256_t result = strider_vec256_cmpeq(a, b);
    uint32_t mask = strider_vec256_movemask(result);

    /* All 32 bits should be set */
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFF, mask);
}

/**
 * Test: Find byte in 256-bit vector
 */
void test_vec256_find_byte(void) {
    memset(test_data_a, 0, 32);
    test_data_a[15] = 'A';
    test_data_a[25] = 'A';

    strider_vec256_t haystack = strider_vec256_load_aligned(test_data_a);
    strider_vec256_t needle = strider_vec256_set1('A');

    strider_vec256_t result = strider_vec256_cmpeq(haystack, needle);
    uint32_t mask = strider_vec256_movemask(result);

    /* Bits 15 and 25 should be set */
    TEST_ASSERT_EQUAL_UINT32((1 << 15) | (1 << 25), mask);
}

/**
 * Test: 256-bit movemask with zeros
 */
void test_vec256_movemask_zeros(void) {
    strider_vec256_t vec = strider_vec256_zero();
    uint32_t mask = strider_vec256_movemask(vec);

    TEST_ASSERT_EQUAL_UINT32(0, mask);
}

#endif /* STRIDER_HAS_AVX2 */

/* ========================================================================
 * Utility Tests
 * ======================================================================== */

/**
 * Test: Count trailing zeros (find first set bit)
 */
void test_count_trailing_zeros(void) {
    /* Test various bit patterns */
    TEST_ASSERT_EQUAL_INT(0, strider_ctz32(1));      /* ...0001 */
    TEST_ASSERT_EQUAL_INT(3, strider_ctz32(8));      /* ...1000 */
    TEST_ASSERT_EQUAL_INT(7, strider_ctz32(128));    /* ...10000000 */
    TEST_ASSERT_EQUAL_INT(15, strider_ctz32(1 << 15));
}

/**
 * Test: Count trailing zeros with zero input
 */
void test_count_trailing_zeros_zero(void) {
    /* Behavior for zero is platform-defined, but should return 32 */
    int result = strider_ctz32(0);
    TEST_ASSERT_EQUAL_INT(32, result);
}

/**
 * Test: Population count (count set bits)
 */
void test_popcount(void) {
    TEST_ASSERT_EQUAL_INT(0, strider_popcount32(0));
    TEST_ASSERT_EQUAL_INT(1, strider_popcount32(1));
    TEST_ASSERT_EQUAL_INT(2, strider_popcount32(3));     /* 0b11 */
    TEST_ASSERT_EQUAL_INT(4, strider_popcount32(15));    /* 0b1111 */
    TEST_ASSERT_EQUAL_INT(16, strider_popcount32(0xFFFF));
    TEST_ASSERT_EQUAL_INT(32, strider_popcount32(0xFFFFFFFF));
}

/* ========================================================================
 * Test Runner
 * ======================================================================== */

int main(void) {
    UNITY_BEGIN();

    /* 128-bit comparison tests */
    RUN_TEST(test_vec128_cmpeq_equal_vectors);
    RUN_TEST(test_vec128_cmpeq_different_vectors);
    RUN_TEST(test_vec128_find_byte);
    RUN_TEST(test_vec128_find_newline);
    RUN_TEST(test_vec128_movemask_all_zeros);
    RUN_TEST(test_vec128_movemask_pattern);

    /* 256-bit comparison tests (AVX2 only) */
    #if defined(STRIDER_HAS_AVX2)
        RUN_TEST(test_vec256_cmpeq_equal_vectors);
        RUN_TEST(test_vec256_find_byte);
        RUN_TEST(test_vec256_movemask_zeros);
    #endif

    /* Utility tests */
    RUN_TEST(test_count_trailing_zeros);
    RUN_TEST(test_count_trailing_zeros_zero);
    RUN_TEST(test_popcount);

    return UNITY_END();
}
