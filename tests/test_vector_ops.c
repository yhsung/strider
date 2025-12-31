/**
 * @file test_vector_ops.c
 * @brief TDD Cycle 3 - Core Vector Abstraction Tests
 *
 * Tests for SIMD vector load/store operations.
 * These tests define the minimal abstraction layer we need.
 *
 * Following TDD:
 * - RED: Write tests that define the API we want
 * - GREEN: Implement minimal code to pass
 * - REFACTOR: Clean up implementation
 */

#include "unity.h"
#include "strider/simd/vector.h"
#include <string.h>
#include <stdlib.h>

/* Test data aligned to 32 bytes (safe for AVX2) */
#if defined(_MSC_VER)
    #define ALIGN_32 __declspec(align(32))
#else
    #define ALIGN_32 __attribute__((aligned(32)))
#endif

static ALIGN_32 uint8_t test_data_aligned[32] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

static ALIGN_32 uint8_t output_buffer[32] = {0};

void setUp(void) {
    /* Clear output buffer before each test */
    memset(output_buffer, 0, sizeof(output_buffer));
}

void tearDown(void) {
    /* Clean up after each test */
}

/* ========================================================================
 * Vector Load Tests (Aligned)
 * ======================================================================== */

/**
 * Test: Load 16 bytes from aligned address (128-bit vector)
 * Expected: Data is correctly loaded into vector
 */
void test_vec128_load_aligned(void) {
    strider_vec128_t vec = strider_vec128_load_aligned(test_data_aligned);

    /* Store back and verify */
    strider_vec128_store_aligned(output_buffer, vec);

    for (int i = 0; i < 16; i++) {
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(test_data_aligned[i], output_buffer[i],
            "Vec128 load should preserve all 16 bytes");
    }
}

/**
 * Test: Load from unaligned address (128-bit)
 * Expected: Works correctly even when address is not aligned
 */
void test_vec128_load_unaligned(void) {
    /* Offset by 3 bytes to create unaligned address */
    const uint8_t* unaligned_ptr = test_data_aligned + 3;

    strider_vec128_t vec = strider_vec128_load_unaligned(unaligned_ptr);
    strider_vec128_store_unaligned(output_buffer, vec);

    for (int i = 0; i < 16; i++) {
        TEST_ASSERT_EQUAL_UINT8(unaligned_ptr[i], output_buffer[i]);
    }
}

/**
 * Test: Set all bytes in vector to same value (broadcast)
 * Expected: All 16 bytes contain the same value
 */
void test_vec128_set1(void) {
    const uint8_t value = 42;
    strider_vec128_t vec = strider_vec128_set1(value);

    strider_vec128_store_aligned(output_buffer, vec);

    for (int i = 0; i < 16; i++) {
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(value, output_buffer[i],
            "All bytes should be set to the broadcast value");
    }
}

/**
 * Test: Create zero vector
 * Expected: All bytes are zero
 */
void test_vec128_zero(void) {
    strider_vec128_t vec = strider_vec128_zero();

    strider_vec128_store_aligned(output_buffer, vec);

    for (int i = 0; i < 16; i++) {
        TEST_ASSERT_EQUAL_UINT8(0, output_buffer[i]);
    }
}

/* ========================================================================
 * Vector Store Tests
 * ======================================================================== */

/**
 * Test: Store to aligned address
 * Expected: All bytes written correctly
 */
void test_vec128_store_aligned(void) {
    strider_vec128_t vec = strider_vec128_load_aligned(test_data_aligned);
    strider_vec128_store_aligned(output_buffer, vec);

    TEST_ASSERT_EQUAL_UINT8_ARRAY(test_data_aligned, output_buffer, 16);
}

/**
 * Test: Store to unaligned address
 * Expected: Works correctly regardless of alignment
 */
void test_vec128_store_unaligned(void) {
    strider_vec128_t vec = strider_vec128_load_aligned(test_data_aligned);

    /* Store at unaligned offset */
    strider_vec128_store_unaligned(output_buffer + 3, vec);

    for (int i = 0; i < 16; i++) {
        TEST_ASSERT_EQUAL_UINT8(test_data_aligned[i], output_buffer[3 + i]);
    }
}

/* ========================================================================
 * 256-bit Vector Tests (if AVX2 available)
 * ======================================================================== */

#if defined(STRIDER_HAS_AVX2) || !defined(STRIDER_ARCH_X86_64)
/**
 * Test: Load 32 bytes (256-bit vector)
 * Expected: All 32 bytes loaded correctly
 */
void test_vec256_load_aligned(void) {
    #if defined(STRIDER_HAS_AVX2)
        strider_vec256_t vec = strider_vec256_load_aligned(test_data_aligned);
        strider_vec256_store_aligned(output_buffer, vec);

        TEST_ASSERT_EQUAL_UINT8_ARRAY(test_data_aligned, output_buffer, 32);
    #else
        TEST_PASS_MESSAGE("AVX2 not available, test skipped");
    #endif
}

/**
 * Test: 256-bit vector set1
 * Expected: All 32 bytes contain same value
 */
void test_vec256_set1(void) {
    #if defined(STRIDER_HAS_AVX2)
        const uint8_t value = 99;
        strider_vec256_t vec = strider_vec256_set1(value);
        strider_vec256_store_aligned(output_buffer, vec);

        for (int i = 0; i < 32; i++) {
            TEST_ASSERT_EQUAL_UINT8(value, output_buffer[i]);
        }
    #else
        TEST_PASS_MESSAGE("AVX2 not available, test skipped");
    #endif
}

/**
 * Test: 256-bit zero vector
 */
void test_vec256_zero(void) {
    #if defined(STRIDER_HAS_AVX2)
        strider_vec256_t vec = strider_vec256_zero();
        strider_vec256_store_aligned(output_buffer, vec);

        for (int i = 0; i < 32; i++) {
            TEST_ASSERT_EQUAL_UINT8(0, output_buffer[i]);
        }
    #else
        TEST_PASS_MESSAGE("AVX2 not available, test skipped");
    #endif
}
#endif

/* ========================================================================
 * Alignment Helpers
 * ======================================================================== */

/**
 * Test: Check if pointer is aligned
 * Expected: Correctly identifies aligned vs unaligned pointers
 */
void test_is_aligned(void) {
    TEST_ASSERT_TRUE(strider_is_aligned(test_data_aligned, 16));
    TEST_ASSERT_TRUE(strider_is_aligned(test_data_aligned, 32));

    /* Unaligned pointer */
    const uint8_t* unaligned = test_data_aligned + 1;
    TEST_ASSERT_FALSE(strider_is_aligned(unaligned, 16));
}

int main(void) {
    UNITY_BEGIN();

    /* 128-bit vector tests (always available) */
    RUN_TEST(test_vec128_load_aligned);
    RUN_TEST(test_vec128_load_unaligned);
    RUN_TEST(test_vec128_set1);
    RUN_TEST(test_vec128_zero);
    RUN_TEST(test_vec128_store_aligned);
    RUN_TEST(test_vec128_store_unaligned);

    /* 256-bit vector tests (AVX2) */
    #if defined(STRIDER_HAS_AVX2) || !defined(STRIDER_ARCH_X86_64)
        RUN_TEST(test_vec256_load_aligned);
        RUN_TEST(test_vec256_set1);
        RUN_TEST(test_vec256_zero);
    #endif

    /* Alignment helpers */
    RUN_TEST(test_is_aligned);

    return UNITY_END();
}
