/**
 * @file test_build.c
 * @brief TDD Cycle 1 - Build system test
 *
 * This is the simplest possible test to verify that:
 * 1. CMake configures correctly
 * 2. The test framework compiles
 * 3. CTest can execute tests
 */

#include "unity.h"

void setUp(void) {
    /* This is run before each test */
}

void tearDown(void) {
    /* This is run after each test */
}

/**
 * Test: Build system works
 * Expected: Always passes (sanity check)
 */
void test_build_system_works(void) {
    TEST_ASSERT_EQUAL(1, 1);
}

/**
 * Test: C standard is correct
 * Expected: C11 or later features available
 */
void test_c_standard(void) {
    /* static_assert is a C11 feature */
    _Static_assert(1, "C11 support");
    TEST_PASS();
}

/**
 * Test: Platform detection
 * Expected: At least one architecture is detected
 */
void test_platform_detection(void) {
#if defined(__x86_64__) || defined(__aarch64__) || defined(_M_X64) || defined(_M_ARM64)
    TEST_PASS_MESSAGE("Platform detected");
#else
    TEST_FAIL_MESSAGE("Unknown platform");
#endif
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_build_system_works);
    RUN_TEST(test_c_standard);
    RUN_TEST(test_platform_detection);

    return UNITY_END();
}
