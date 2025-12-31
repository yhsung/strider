/**
 * @file test_newline.c
 * @brief Unit tests for newline detection and counting
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2025 Strider Development Team
 *
 * TDD Cycle 7: Newline Detection
 * Tests both scalar and SIMD implementations of newline counting.
 */

#include "strider/parsers/newline.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

void setUp(void) {
    /* Setup before each test */
}

void tearDown(void) {
    /* Cleanup after each test */
}

/* ========================================================================
 * Basic Newline Counting Tests
 * ======================================================================== */

/**
 * Test: Count Unix newlines (\n only)
 */
void test_find_newlines_unix(void) {
    const char *text = "line 1\nline 2\nline 3\n";

    size_t count = strider_count_newlines(text, strlen(text));

    TEST_ASSERT_EQUAL_size_t(3, count);
}

/**
 * Test: Count Windows newlines (\r\n pairs)
 */
void test_find_newlines_windows(void) {
    const char *text = "line 1\r\nline 2\r\nline 3\r\n";

    size_t count = strider_count_newlines(text, strlen(text));

    /* Should count \r\n as single newline */
    TEST_ASSERT_EQUAL_size_t(3, count);
}

/**
 * Test: Count Mac Classic newlines (\r only)
 */
void test_find_newlines_mac_classic(void) {
    const char *text = "line 1\rline 2\rline 3\r";

    size_t count = strider_count_newlines(text, strlen(text));

    TEST_ASSERT_EQUAL_size_t(3, count);
}

/**
 * Test: Handle mixed newline styles
 */
void test_find_newlines_mixed(void) {
    const char *text = "unix\nwindows\r\nmac\rmixed\n\r";

    size_t count = strider_count_newlines(text, strlen(text));

    /* unix(1) + windows(1) + mac(1) + final mixed(2) = 5 */
    TEST_ASSERT_EQUAL_size_t(5, count);
}

/**
 * Test: Handle consecutive newlines
 */
void test_find_newlines_consecutive(void) {
    const char *text = "line 1\n\n\nline 2\r\n\r\nline 3";

    size_t count = strider_count_newlines(text, strlen(text));

    /* 3 \n + 2 \r\n = 5 newlines */
    TEST_ASSERT_EQUAL_size_t(5, count);
}

/**
 * Test: No newlines present
 */
void test_find_newlines_none(void) {
    const char *text = "This is a single line with no newlines";

    size_t count = strider_count_newlines(text, strlen(text));

    TEST_ASSERT_EQUAL_size_t(0, count);
}

/**
 * Test: Empty string
 */
void test_find_newlines_empty(void) {
    const char *text = "";

    size_t count = strider_count_newlines(text, 0);

    TEST_ASSERT_EQUAL_size_t(0, count);
}

/* ========================================================================
 * Newline Position Tracking Tests
 * ======================================================================== */

/**
 * Test: Return array of newline positions
 */
void test_find_newlines_offsets(void) {
    const char *text = "aa\nbb\ncc\n";
    size_t positions[10];
    size_t count;

    count = strider_find_newline_positions(text, strlen(text), positions, 10);

    TEST_ASSERT_EQUAL_size_t(3, count);
    TEST_ASSERT_EQUAL_size_t(2, positions[0]); /* \n at index 2 */
    TEST_ASSERT_EQUAL_size_t(5, positions[1]); /* \n at index 5 */
    TEST_ASSERT_EQUAL_size_t(8, positions[2]); /* \n at index 8 */
}

/**
 * Test: Position tracking with limited buffer
 */
void test_find_newlines_offsets_limited(void) {
    const char *text = "1\n2\n3\n4\n5\n";
    size_t positions[3]; /* Only space for 3 */
    size_t count;

    count = strider_find_newline_positions(text, strlen(text), positions, 3);

    /* Should return total count (5), but only store first 3 positions */
    TEST_ASSERT_EQUAL_size_t(5, count);
    TEST_ASSERT_EQUAL_size_t(1, positions[0]);
    TEST_ASSERT_EQUAL_size_t(3, positions[1]);
    TEST_ASSERT_EQUAL_size_t(5, positions[2]);
}

/* ========================================================================
 * SIMD Implementation Tests
 * ======================================================================== */

/**
 * Test: SIMD vectorized counting on aligned data
 */
void test_newlines_simd_vectorized(void) {
    /* Create 128-byte buffer with newlines */
    char *buffer = (char *) strider_aligned_alloc(32, 128);
    TEST_ASSERT_NOT_NULL(buffer);

    memset(buffer, 'X', 128);
    /* Add newlines at regular intervals */
    buffer[15] = '\n';
    buffer[31] = '\n';
    buffer[63] = '\n';
    buffer[95] = '\n';
    buffer[127] = '\n';

    size_t count = strider_count_newlines_simd(buffer, 128);

    TEST_ASSERT_EQUAL_size_t(5, count);

    strider_aligned_free(buffer);
}

/**
 * Test: SIMD with unaligned buffer
 */
void test_newlines_simd_unaligned(void) {
    char buffer[100];

    /* Fill with data */
    for (size_t i = 0; i < 100; i++) {
        buffer[i] = 'A' + (i % 26);
    }
    buffer[10] = '\n';
    buffer[25] = '\n';
    buffer[50] = '\n';
    buffer[75] = '\n';
    buffer[99] = '\n';

    size_t count = strider_count_newlines_simd(buffer, 100);

    TEST_ASSERT_EQUAL_size_t(5, count);
}

/**
 * Test: SIMD matches scalar implementation
 */
void test_newlines_simd_matches_scalar(void) {
    const char *test_strings[] = {
        "no newlines here",
        "one\n",
        "two\nlines\n",
        "unix\nwindows\r\nmac\r",
        "\n\n\n\n\n",
        "text\r\n\r\nmore\r\n",
        "",
        "A",
    };
    size_t num_tests = sizeof(test_strings) / sizeof(test_strings[0]);

    for (size_t i = 0; i < num_tests; i++) {
        size_t len = strlen(test_strings[i]);
        size_t scalar_count = strider_count_newlines(test_strings[i], len);
        size_t simd_count = strider_count_newlines_simd(test_strings[i], len);

        TEST_ASSERT_EQUAL_size_t_MESSAGE(scalar_count, simd_count,
                                         "SIMD should match scalar count");
    }
}

/* ========================================================================
 * Large Buffer Tests
 * ======================================================================== */

/**
 * Test: Large buffer (1MB+)
 */
void test_find_newlines_large_buffer(void) {
    /* Create 1MB buffer */
    size_t size = 1024 * 1024;
    char *buffer = (char *) malloc(size);
    TEST_ASSERT_NOT_NULL(buffer);

    /* Fill with text, newline every 80 bytes (like log lines) */
    size_t expected_newlines = 0;
    for (size_t i = 0; i < size; i++) {
        if (i % 80 == 79) {
            buffer[i] = '\n';
            expected_newlines++;
        } else {
            buffer[i] = 'A' + (i % 26);
        }
    }

    /* Test both implementations */
    size_t scalar_count = strider_count_newlines(buffer, size);
    size_t simd_count = strider_count_newlines_simd(buffer, size);

    TEST_ASSERT_EQUAL_size_t(expected_newlines, scalar_count);
    TEST_ASSERT_EQUAL_size_t(expected_newlines, simd_count);

    free(buffer);
}

/**
 * Test: Match wc -l behavior (count newlines, not lines)
 */
void test_newlines_matches_wc(void) {
    /* wc -l counts newlines, not lines
     * "abc" = 0 newlines
     * "abc\n" = 1 newline
     * "abc\ndef" = 1 newline
     * "abc\ndef\n" = 2 newlines
     */

    TEST_ASSERT_EQUAL_size_t(0, strider_count_newlines("abc", 3));
    TEST_ASSERT_EQUAL_size_t(1, strider_count_newlines("abc\n", 4));
    TEST_ASSERT_EQUAL_size_t(1, strider_count_newlines("abc\ndef", 7));
    TEST_ASSERT_EQUAL_size_t(2, strider_count_newlines("abc\ndef\n", 8));
}

/* ========================================================================
 * Test Runner
 * ======================================================================== */

int main(void) {
    UNITY_BEGIN();

    /* Basic counting tests */
    RUN_TEST(test_find_newlines_unix);
    RUN_TEST(test_find_newlines_windows);
    RUN_TEST(test_find_newlines_mac_classic);
    RUN_TEST(test_find_newlines_mixed);
    RUN_TEST(test_find_newlines_consecutive);
    RUN_TEST(test_find_newlines_none);
    RUN_TEST(test_find_newlines_empty);

    /* Position tracking tests */
    RUN_TEST(test_find_newlines_offsets);
    RUN_TEST(test_find_newlines_offsets_limited);

    /* SIMD tests */
    RUN_TEST(test_newlines_simd_vectorized);
    RUN_TEST(test_newlines_simd_unaligned);
    RUN_TEST(test_newlines_simd_matches_scalar);

    /* Large buffer and compatibility tests */
    RUN_TEST(test_find_newlines_large_buffer);
    RUN_TEST(test_newlines_matches_wc);

    return UNITY_END();
}
