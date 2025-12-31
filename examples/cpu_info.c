/**
 * @file cpu_info.c
 * @brief Example: Display detected CPU features
 *
 * Demonstrates the use of strider_get_cpu_features() to query
 * available SIMD instructions at runtime.
 */

#include <stdio.h>
#include "strider/config.h"

int main(void) {
    printf("Strider CPU Feature Detection\n");
    printf("==============================\n\n");

    strider_cpu_features_t features = strider_get_cpu_features();

    /* Display features in human-readable format */
    char buffer[1024];
    strider_describe_cpu_features(&features, buffer, sizeof(buffer));
    printf("%s\n", buffer);

    /* Display compile-time vs runtime info */
    printf("Compile-Time Configuration:\n");
    printf("  Architecture: %s\n", STRIDER_ARCH_NAME);

    #ifdef STRIDER_HAS_SSE2
        printf("  - SSE2 (compiled in)\n");
    #endif
    #ifdef STRIDER_HAS_AVX2
        printf("  - AVX2 (compiled in)\n");
    #endif
    #ifdef STRIDER_HAS_AVX512F
        printf("  - AVX-512 (compiled in)\n");
    #endif
    #ifdef STRIDER_HAS_NEON
        printf("  - NEON (compiled in)\n");
    #endif

    return 0;
}
