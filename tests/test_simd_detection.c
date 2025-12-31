/**
 * @file test_simd_detection.c
 * @brief TDD Cycle 2 - SIMD Feature Detection Tests
 *
 * Tests for runtime CPU feature detection across x86_64 and ARM64 platforms.
 * Following RED-GREEN-REFACTOR:
 * - RED: These tests will fail until we implement config.h
 * - GREEN: Implement minimal detection code
 * - REFACTOR: Clean up implementation
 */

#include "unity.h"
#include "strider/config.h"

void setUp(void) {
    /* Run before each test */
}

void tearDown(void) {
    /* Run after each test */
}

/**
 * Test: CPU features can be queried
 * Expected: Function returns a valid features struct
 */
void test_cpu_features_query(void) {
    strider_cpu_features_t features = strider_get_cpu_features();

    /* At least one architecture should be detected */
    #if defined(__x86_64__) || defined(_M_X64)
        TEST_ASSERT_TRUE(features.arch_x86_64);
        TEST_ASSERT_FALSE(features.arch_arm64);
    #elif defined(__aarch64__) || defined(_M_ARM64)
        TEST_ASSERT_TRUE(features.arch_arm64);
        TEST_ASSERT_FALSE(features.arch_x86_64);
    #endif
}

/**
 * Test: SSE2 detection on x86_64
 * Expected: SSE2 is available (baseline requirement for x86_64)
 */
void test_cpu_has_sse2(void) {
    #if defined(__x86_64__) || defined(_M_X64)
        strider_cpu_features_t features = strider_get_cpu_features();
        TEST_ASSERT_TRUE_MESSAGE(features.has_sse2,
            "SSE2 should be available on all x86_64 CPUs");
    #else
        TEST_PASS_MESSAGE("SSE2 test skipped on non-x86 platform");
    #endif
}

/**
 * Test: AVX2 detection on x86_64
 * Expected: Detects AVX2 if available (may not be on all systems)
 */
void test_cpu_has_avx2(void) {
    #if defined(__x86_64__) || defined(_M_X64)
        strider_cpu_features_t features = strider_get_cpu_features();

        /* Just verify the field exists and is boolean */
        TEST_ASSERT_TRUE(features.has_avx2 == 0 || features.has_avx2 == 1);

        #ifdef __AVX2__
            /* If compiled with AVX2, it should be detected */
            TEST_ASSERT_TRUE_MESSAGE(features.has_avx2,
                "AVX2 compiled but not detected");
        #endif
    #else
        TEST_PASS_MESSAGE("AVX2 test skipped on non-x86 platform");
    #endif
}

/**
 * Test: AVX-512 detection on x86_64
 * Expected: Detects AVX-512 if available (rare, usually server CPUs)
 */
void test_cpu_has_avx512(void) {
    #if defined(__x86_64__) || defined(_M_X64)
        strider_cpu_features_t features = strider_get_cpu_features();

        /* Just verify the field exists and is boolean */
        TEST_ASSERT_TRUE(features.has_avx512f == 0 || features.has_avx512f == 1);

        #ifdef __AVX512F__
            TEST_ASSERT_TRUE_MESSAGE(features.has_avx512f,
                "AVX-512 compiled but not detected");
        #endif
    #else
        TEST_PASS_MESSAGE("AVX-512 test skipped on non-x86 platform");
    #endif
}

/**
 * Test: NEON detection on ARM64
 * Expected: NEON is available (standard on ARM64)
 */
void test_cpu_has_neon(void) {
    #if defined(__aarch64__) || defined(_M_ARM64)
        strider_cpu_features_t features = strider_get_cpu_features();
        TEST_ASSERT_TRUE_MESSAGE(features.has_neon,
            "NEON should be available on all ARM64 CPUs");
    #else
        TEST_PASS_MESSAGE("NEON test skipped on non-ARM platform");
    #endif
}

/**
 * Test: Compile-time SIMD macros are defined
 * Expected: At least one SIMD feature is available at compile-time
 */
void test_compile_time_simd_macros(void) {
    #if defined(__x86_64__) || defined(_M_X64)
        /* x86_64 should have at least SSE2 */
        #if defined(STRIDER_HAS_SSE2) || defined(__SSE2__)
            TEST_PASS_MESSAGE("SSE2 available at compile-time");
        #else
            TEST_FAIL_MESSAGE("No SIMD support detected at compile-time for x86_64");
        #endif
    #elif defined(__aarch64__) || defined(_M_ARM64)
        /* ARM64 should have NEON */
        #if defined(STRIDER_HAS_NEON) || defined(__ARM_NEON)
            TEST_PASS_MESSAGE("NEON available at compile-time");
        #else
            TEST_FAIL_MESSAGE("No SIMD support detected at compile-time for ARM64");
        #endif
    #else
        TEST_FAIL_MESSAGE("Unknown architecture");
    #endif
}

/**
 * Test: Feature detection is consistent
 * Expected: Calling twice returns same results
 */
void test_feature_detection_is_consistent(void) {
    strider_cpu_features_t features1 = strider_get_cpu_features();
    strider_cpu_features_t features2 = strider_get_cpu_features();

    TEST_ASSERT_EQUAL(features1.arch_x86_64, features2.arch_x86_64);
    TEST_ASSERT_EQUAL(features1.arch_arm64, features2.arch_arm64);

    #if defined(__x86_64__) || defined(_M_X64)
        TEST_ASSERT_EQUAL(features1.has_sse2, features2.has_sse2);
        TEST_ASSERT_EQUAL(features1.has_avx2, features2.has_avx2);
        TEST_ASSERT_EQUAL(features1.has_avx512f, features2.has_avx512f);
    #elif defined(__aarch64__) || defined(_M_ARM64)
        TEST_ASSERT_EQUAL(features1.has_neon, features2.has_neon);
    #endif
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_cpu_features_query);
    RUN_TEST(test_cpu_has_sse2);
    RUN_TEST(test_cpu_has_avx2);
    RUN_TEST(test_cpu_has_avx512);
    RUN_TEST(test_cpu_has_neon);
    RUN_TEST(test_compile_time_simd_macros);
    RUN_TEST(test_feature_detection_is_consistent);

    return UNITY_END();
}
