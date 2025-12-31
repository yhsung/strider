/**
 * @file test_strchr.c
 * @brief Unit tests for single character search (strchr-like operations)
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2025 Strider Development Team
 *
 * TDD Cycle 6: Single Character Search
 * Tests both scalar reference and SIMD implementations of strchr functionality.
 */

#include "strider/parsers/strchr.h"
#include "unity.h"
#include <string.h>

void setUp(void) {
    /* Setup before each test */
}

void tearDown(void) {
    /* Cleanup after each test */
}

/* ========================================================================
 * Scalar Reference Implementation Tests
 * ======================================================================== */

/**
 * Test: Find character at start of string
 */
void test_strchr_found_first(void) {
    const char *str = "Hello, world!";
    const char *result = strider_strchr(str, 'H');

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(str, result);
    TEST_ASSERT_EQUAL_CHAR('H', *result);
}

/**
 * Test: Find character in middle of string
 */
void test_strchr_found_middle(void) {
    const char *str = "Hello, world!";
    const char *result = strider_strchr(str, 'w');

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(str + 7, result);
    TEST_ASSERT_EQUAL_CHAR('w', *result);
}

/**
 * Test: Find character at end of string
 */
void test_strchr_found_last(void) {
    const char *str = "Hello, world!";
    const char *result = strider_strchr(str, '!');

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(str + 12, result);
    TEST_ASSERT_EQUAL_CHAR('!', *result);
}

/**
 * Test: Character not present in string
 */
void test_strchr_not_found(void) {
    const char *str = "Hello, world!";
    const char *result = strider_strchr(str, 'X');

    TEST_ASSERT_NULL(result);
}

/**
 * Test: Search in empty string
 */
void test_strchr_empty_string(void) {
    const char *str = "";
    const char *result = strider_strchr(str, 'A');

    TEST_ASSERT_NULL(result);
}

/**
 * Test: Search for null terminator
 */
void test_strchr_null_terminator(void) {
    const char *str = "Hello";
    const char *result = strider_strchr(str, '\0');

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(str + 5, result);
    TEST_ASSERT_EQUAL_CHAR('\0', *result);
}

/**
 * Test: Multiple occurrences - should return first match
 */
void test_strchr_multiple_occurrences(void) {
    const char *str = "Mississippi";
    const char *result = strider_strchr(str, 's');

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(str + 2, result); /* First 's' at index 2 */
    TEST_ASSERT_EQUAL_CHAR('s', *result);
}

/* ========================================================================
 * SIMD Implementation Tests
 * ======================================================================== */

/**
 * Test: SIMD search in aligned buffer
 */
void test_strchr_simd_aligned(void) {
    /* Allocate 32-byte aligned buffer */
    char *buffer = (char *) strider_aligned_alloc(32, 64);
    TEST_ASSERT_NOT_NULL(buffer);

    strcpy(buffer, "This is an aligned test string for SIMD operations!");

    const char *result = strider_strchr_simd(buffer, 'S');

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_CHAR('S', *result);

    /* Verify it's the same position as scalar */
    const char *scalar_result = strider_strchr(buffer, 'S');
    TEST_ASSERT_EQUAL_PTR(scalar_result, result);

    strider_aligned_free(buffer);
}

/**
 * Test: SIMD search with unaligned prefix
 */
void test_strchr_simd_unaligned_prefix(void) {
    /* Create buffer where target is in unaligned prefix */
    char buffer[64];
    strcpy(buffer, "X at position 0");

    const char *result = strider_strchr_simd(buffer, 'X');

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(buffer, result);
}

/**
 * Test: SIMD search with unaligned suffix
 */
void test_strchr_simd_unaligned_suffix(void) {
    /* Create buffer where target is in unaligned suffix */
    char buffer[100];
    memset(buffer, 'A', 99);
    buffer[98] = 'Z'; /* Put target near end */
    buffer[99] = '\0';

    const char *result = strider_strchr_simd(buffer, 'Z');

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(buffer + 98, result);
}

/**
 * Test: SIMD matches scalar on random inputs (fuzz test)
 */
void test_strchr_simd_matches_scalar(void) {
    const char *test_strings[] = {"a",
                                  "ab",
                                  "abc",
                                  "The quick brown fox jumps over the lazy dog",
                                  "!@#$%^&*()_+-=[]{}|;:',.<>?/",
                                  "1234567890"
                                  "1234567890"
                                  "1234567890"
                                  "1234567890", /* 40 chars */
                                  "MixedCASE with SPACES and numbers 12345",
                                  ""};
    const size_t num_tests = sizeof(test_strings) / sizeof(test_strings[0]);

    const char search_chars[] = {'a', 'z', 'X', ' ', '1', '\0', '@', 'q'};
    const size_t num_chars = sizeof(search_chars) / sizeof(search_chars[0]);

    for (size_t i = 0; i < num_tests; i++) {
        for (size_t j = 0; j < num_chars; j++) {
            const char *scalar_result = strider_strchr(test_strings[i], search_chars[j]);
            const char *simd_result = strider_strchr_simd(test_strings[i], search_chars[j]);

            /* Both should return same result */
            if (scalar_result == NULL) {
                TEST_ASSERT_NULL_MESSAGE(simd_result, "SIMD should match scalar NULL result");
            } else {
                TEST_ASSERT_EQUAL_PTR_MESSAGE(scalar_result, simd_result,
                                              "SIMD should match scalar pointer");
            }
        }
    }
}

/**
 * Test: Long string search (performance validation)
 */
void test_strchr_long_string(void) {
    /* Create a long string */
    size_t len = 10000;
    char *long_str = (char *) malloc(len + 1);
    TEST_ASSERT_NOT_NULL(long_str);

    /* Fill with pattern that doesn't include '0' digit */
    for (size_t i = 0; i < len; i++) {
        long_str[i] = 'A' + (i % 26);
    }
    long_str[5000] = '0'; /* Target in middle - digit not in A-Z pattern */
    long_str[len] = '\0';

    /* Test both implementations */
    const char *scalar_result = strider_strchr(long_str, '0');
    const char *simd_result = strider_strchr_simd(long_str, '0');

    TEST_ASSERT_NOT_NULL(scalar_result);
    TEST_ASSERT_NOT_NULL(simd_result);
    TEST_ASSERT_EQUAL_PTR(scalar_result, simd_result);
    TEST_ASSERT_EQUAL_PTR(long_str + 5000, simd_result);

    free(long_str);
}

/**
 * Test: Search for character not in long string
 */
void test_strchr_long_string_not_found(void) {
    size_t len = 10000;
    char *long_str = (char *) malloc(len + 1);
    TEST_ASSERT_NOT_NULL(long_str);

    /* Fill with 'A' only */
    memset(long_str, 'A', len);
    long_str[len] = '\0';

    /* Search for character not present */
    const char *scalar_result = strider_strchr(long_str, 'Z');
    const char *simd_result = strider_strchr_simd(long_str, 'Z');

    TEST_ASSERT_NULL(scalar_result);
    TEST_ASSERT_NULL(simd_result);

    free(long_str);
}

/* ========================================================================
 * Test Runner
 * ======================================================================== */

int main(void) {
    UNITY_BEGIN();

    /* Scalar reference tests */
    RUN_TEST(test_strchr_found_first);
    RUN_TEST(test_strchr_found_middle);
    RUN_TEST(test_strchr_found_last);
    RUN_TEST(test_strchr_not_found);
    RUN_TEST(test_strchr_empty_string);
    RUN_TEST(test_strchr_null_terminator);
    RUN_TEST(test_strchr_multiple_occurrences);

    /* SIMD implementation tests */
    RUN_TEST(test_strchr_simd_aligned);
    RUN_TEST(test_strchr_simd_unaligned_prefix);
    RUN_TEST(test_strchr_simd_unaligned_suffix);
    RUN_TEST(test_strchr_simd_matches_scalar);
    RUN_TEST(test_strchr_long_string);
    RUN_TEST(test_strchr_long_string_not_found);

    return UNITY_END();
}
