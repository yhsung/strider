/**
 * @file config.c
 * @brief CPU feature detection implementation
 *
 * Runtime detection of SIMD capabilities using:
 * - CPUID on x86_64
 * - getauxval/sysctlbyname on ARM64
 */

#include "strider/config.h"
#include <string.h>
#include <stdio.h>

#if defined(STRIDER_ARCH_X86_64)
    #if defined(_MSC_VER)
        #include <intrin.h>
    #else
        #include <cpuid.h>
    #endif
#elif defined(STRIDER_ARCH_ARM64)
    #if defined(__linux__)
        #include <sys/auxv.h>
        #include <asm/hwcap.h>
    #elif defined(__APPLE__)
        #include <sys/types.h>
        #include <sys/sysctl.h>
    #endif
#endif

/* ========================================================================
 * x86_64 CPUID Helper Functions
 * ======================================================================== */

#if defined(STRIDER_ARCH_X86_64)

static void cpuid(uint32_t leaf, uint32_t subleaf,
                   uint32_t* eax, uint32_t* ebx,
                   uint32_t* ecx, uint32_t* edx) {
    #if defined(_MSC_VER)
        int regs[4];
        __cpuidex(regs, leaf, subleaf);
        *eax = regs[0];
        *ebx = regs[1];
        *ecx = regs[2];
        *edx = regs[3];
    #else
        __cpuid_count(leaf, subleaf, *eax, *ebx, *ecx, *edx);
    #endif
}

static void detect_x86_features(strider_cpu_features_t* features) {
    uint32_t eax, ebx, ecx, edx;

    /* Get vendor string */
    cpuid(0, 0, &eax, &ebx, &ecx, &edx);
    memcpy(features->vendor + 0, &ebx, 4);
    memcpy(features->vendor + 4, &edx, 4);
    memcpy(features->vendor + 8, &ecx, 4);
    features->vendor[12] = '\0';

    /* Get CPU family and model */
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    features->family = (eax >> 8) & 0xF;
    features->model = (eax >> 4) & 0xF;

    /* Detect SSE/AVX features from CPUID leaf 1 */
    features->has_sse2 = (edx & (1 << 26)) != 0;
    features->has_sse3 = (ecx & (1 << 0)) != 0;
    features->has_ssse3 = (ecx & (1 << 9)) != 0;
    features->has_sse4_1 = (ecx & (1 << 19)) != 0;
    features->has_sse4_2 = (ecx & (1 << 20)) != 0;
    features->has_avx = (ecx & (1 << 28)) != 0;

    /* Detect AVX2 and AVX-512 from CPUID leaf 7 */
    cpuid(7, 0, &eax, &ebx, &ecx, &edx);
    features->has_avx2 = (ebx & (1 << 5)) != 0;
    features->has_avx512f = (ebx & (1 << 16)) != 0;
    features->has_avx512bw = (ebx & (1 << 30)) != 0;
}

#endif /* STRIDER_ARCH_X86_64 */

/* ========================================================================
 * ARM64 Feature Detection
 * ======================================================================== */

#if defined(STRIDER_ARCH_ARM64)

static void detect_arm_features(strider_cpu_features_t* features) {
    /* NEON is standard on ARM64 */
    features->has_neon = true;

    #if defined(__linux__)
        /* Use getauxval to detect advanced features */
        unsigned long hwcap = getauxval(AT_HWCAP);

        #ifdef HWCAP_ASIMD
            features->has_neon = (hwcap & HWCAP_ASIMD) != 0;
        #endif

        #ifdef HWCAP_SVE
            features->has_sve = (hwcap & HWCAP_SVE) != 0;
        #endif

        #ifdef HWCAP2_SVE2
            unsigned long hwcap2 = getauxval(AT_HWCAP2);
            features->has_sve2 = (hwcap2 & HWCAP2_SVE2) != 0;
        #endif

    #elif defined(__APPLE__)
        /* macOS: NEON is always available on ARM64 */
        features->has_neon = true;

        /* Check for optional features via sysctl */
        int has_feature = 0;
        size_t size = sizeof(has_feature);

        if (sysctlbyname("hw.optional.arm.FEAT_SVE", &has_feature, &size, NULL, 0) == 0) {
            features->has_sve = (has_feature != 0);
        }
    #endif

    strcpy(features->vendor, "ARM");
}

#endif /* STRIDER_ARCH_ARM64 */

/* ========================================================================
 * Public API
 * ======================================================================== */

strider_cpu_features_t strider_get_cpu_features(void) {
    /* Cache features after first detection */
    static strider_cpu_features_t cached_features = {0};
    static bool initialized = false;

    if (initialized) {
        return cached_features;
    }

    /* Initialize all fields to false/zero */
    memset(&cached_features, 0, sizeof(cached_features));

    /* Detect architecture */
    #if defined(STRIDER_ARCH_X86_64)
        cached_features.arch_x86_64 = true;
        detect_x86_features(&cached_features);
    #elif defined(STRIDER_ARCH_ARM64)
        cached_features.arch_arm64 = true;
        detect_arm_features(&cached_features);
    #endif

    initialized = true;
    return cached_features;
}

int strider_describe_cpu_features(const strider_cpu_features_t* features,
                                    char* buffer, size_t buffer_size) {
    if (!features || !buffer || buffer_size == 0) {
        return -1;
    }

    int written = 0;

    /* Architecture */
    written += snprintf(buffer + written, buffer_size - written,
                        "Architecture: %s\n",
                        features->arch_x86_64 ? "x86_64" : "ARM64");

    /* Vendor */
    if (features->vendor[0]) {
        written += snprintf(buffer + written, buffer_size - written,
                            "Vendor: %s\n", features->vendor);
    }

    /* SIMD features */
    written += snprintf(buffer + written, buffer_size - written, "SIMD Features:\n");

    #if defined(STRIDER_ARCH_X86_64)
        if (features->has_sse2) written += snprintf(buffer + written, buffer_size - written, "  - SSE2\n");
        if (features->has_sse3) written += snprintf(buffer + written, buffer_size - written, "  - SSE3\n");
        if (features->has_ssse3) written += snprintf(buffer + written, buffer_size - written, "  - SSSE3\n");
        if (features->has_sse4_1) written += snprintf(buffer + written, buffer_size - written, "  - SSE4.1\n");
        if (features->has_sse4_2) written += snprintf(buffer + written, buffer_size - written, "  - SSE4.2\n");
        if (features->has_avx) written += snprintf(buffer + written, buffer_size - written, "  - AVX\n");
        if (features->has_avx2) written += snprintf(buffer + written, buffer_size - written, "  - AVX2\n");
        if (features->has_avx512f) written += snprintf(buffer + written, buffer_size - written, "  - AVX-512F\n");
        if (features->has_avx512bw) written += snprintf(buffer + written, buffer_size - written, "  - AVX-512BW\n");
    #elif defined(STRIDER_ARCH_ARM64)
        if (features->has_neon) written += snprintf(buffer + written, buffer_size - written, "  - NEON\n");
        if (features->has_sve) written += snprintf(buffer + written, buffer_size - written, "  - SVE\n");
        if (features->has_sve2) written += snprintf(buffer + written, buffer_size - written, "  - SVE2\n");
    #endif

    return written;
}
