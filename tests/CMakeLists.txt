# Unity test framework (single-file, embedded)
add_library(unity STATIC unity/unity.c)
target_include_directories(unity PUBLIC unity)

# Helper function to create tests
function(add_strider_test TEST_NAME)
    add_executable(${TEST_NAME} ${ARGN})
    target_link_libraries(${TEST_NAME} PRIVATE strider unity)

    # Enable sanitizers if requested
    if(STRIDER_ENABLE_ASAN)
        target_compile_options(${TEST_NAME} PRIVATE -fsanitize=address)
        target_link_options(${TEST_NAME} PRIVATE -fsanitize=address)
    endif()

    if(STRIDER_ENABLE_UBSAN)
        target_compile_options(${TEST_NAME} PRIVATE -fsanitize=undefined)
        target_link_options(${TEST_NAME} PRIVATE -fsanitize=undefined)
    endif()

    # Add to CTest
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# TDD Cycle 1: First test - build system works
add_strider_test(test_build test_build.c)

# TDD Cycle 2: SIMD feature detection
add_strider_test(test_simd_detection test_simd_detection.c)

# TDD Cycle 3: Core vector abstraction - load/store
add_strider_test(test_vector_ops test_vector_ops.c)

# TDD Cycle 4: Vector comparison operations
add_strider_test(test_vector_compare test_vector_compare.c)

# TDD Cycle 5: Memory utilities
add_strider_test(test_memory_utils test_memory_utils.c)

# TDD Cycle 6: Single character search
add_strider_test(test_strchr test_strchr.c)

# TDD Cycle 7: Newline detection
add_strider_test(test_newline test_newline.c)
