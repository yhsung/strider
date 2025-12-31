# Strider ⚡ Development Plan (TDD Approach)

## Project Overview

**Project Name:** Strider
**Type:** C Library for Vectorized String/Log Parsing
**Purpose:** High-performance string log processing via SIMD abstraction
**Development Methodology:** Test-Driven Development (TDD)

Strider is a Proof of Concept (POC) demonstrating how to accelerate common log-parsing tasks (like newline detection, timestamp extraction, and level filtering) using hardware-level parallelism.

### Key Features
- **Multi-Arch Support**: Unified interface for x86_64 (AVX2/AVX-512) and ARM64 (NEON)
- **Vectorized Search**: Uses SIMD instructions to scan megabytes of log data in a single pass
- **Zero-Copy**: Designed for data-oriented workflows to minimize memory overhead
- **Header-Only Abstraction**: Minimal runtime overhead with compile-time optimization
- **Test-First Development**: Every feature developed using Red-Green-Refactor cycle

---

## Architecture Overview

### Core Components

```
strider/
├── include/
│   └── strider/
│       ├── strider.h              # Main public API
│       ├── config.h               # Build configuration & feature detection
│       ├── simd/
│       │   ├── vector.h           # SIMD abstraction layer
│       │   ├── x86/
│       │   │   ├── avx2.h        # AVX2 implementations
│       │   │   └── avx512.h      # AVX-512 implementations
│       │   └── arm/
│       │       └── neon.h        # NEON implementations
│       ├── parsers/
│       │   ├── newline.h         # Newline detection
│       │   ├── timestamp.h       # Timestamp extraction
│       │   └── level.h           # Log level filtering
│       └── utils/
│           ├── memory.h          # Memory utilities (alignment, etc.)
│           └── string.h          # String utilities
├── src/
│   └── (implementation files if not header-only)
├── tests/
│   ├── unit/                     # Unit tests per component
│   ├── benchmark/                # Performance benchmarks
│   └── integration/              # End-to-end tests
├── examples/
│   ├── basic_parsing.c
│   ├── log_filter.c
│   └── benchmark_demo.c
└── docs/
    ├── API.md
    ├── BENCHMARKS.md
    └── plans/
        └── DEVELOPMENT_PLAN.md
```

### SIMD Abstraction Strategy

The library uses a unified interface that maps to architecture-specific intrinsics:

**x86_64:**
- AVX2: `_mm256_cmpeq_epi8`, `_mm256_movemask_epi8`
- AVX-512: `_mm512_cmpeq_epi8_mask`, `_mm512_mask_compressstoreu_epi8`

**ARM64:**
- NEON: `vceqq_u8`, `vaddv_u8`, `vget_lane_u64`

**Abstraction Example:**
```c
// Unified vector type
typedef struct {
    #if defined(__AVX2__)
        __m256i data;
    #elif defined(__ARM_NEON)
        uint8x16_t data;
    #endif
} vec_t;

// Unified operations
vec_t vec_load(const char* ptr);
vec_t vec_cmpeq(vec_t a, vec_t b);
uint32_t vec_mask(vec_t v);
```

---

## TDD Workflow

Every feature follows the **Red-Green-Refactor** cycle:

1. **RED**: Write a failing test that defines desired behavior
2. **GREEN**: Write minimal code to make the test pass
3. **REFACTOR**: Improve code quality while keeping tests green
4. **REPEAT**: Add next test case and continue

### TDD Principles for Strider
- Write tests before implementation code
- One test case at a time, starting with simplest scenarios
- Test both scalar and SIMD implementations independently
- Validate correctness of SIMD code against scalar reference
- Write benchmarks as part of the test suite
- Use property-based testing for edge cases

---

## Development Phases

### Phase 1: Foundation & Core Infrastructure (TDD)
**Goal:** Establish build system, SIMD detection, and basic abstractions
**Approach:** Test-first development of all foundational components

#### TDD Cycle 1: Build System & Test Framework
**Tests to write first:**
1. Test that CMake detects current platform correctly
2. Test that compiler flags enable SIMD instructions
3. Test that test runner can execute and report results
4. Test that build fails if SIMD not available (where required)

**Implementation:**
- Configure CMake with CTest integration
- Add Unity/Criterion/cmocka test framework
- Set up test discovery and execution
- Create `test/test_main.c` skeleton

#### TDD Cycle 2: SIMD Feature Detection
**Tests to write first:**
1. `test_cpu_has_sse2()` - Verify SSE2 detection (baseline x86)
2. `test_cpu_has_avx2()` - Verify AVX2 detection
3. `test_cpu_has_avx512()` - Verify AVX-512 detection
4. `test_cpu_has_neon()` - Verify NEON detection (ARM)
5. `test_compile_time_simd_macros()` - Verify compile-time flags

**Implementation:**
- Runtime CPU feature detection (CPUID for x86, hwcap for ARM)
- Create `include/strider/config.h` with detection functions
- Implement `strider_cpu_features()` API
- Fallback mechanism selection

#### TDD Cycle 3: Core Vector Abstraction - Load/Store
**Tests to write first:**
1. `test_vec_load_aligned()` - Load from 16/32-byte aligned address
2. `test_vec_load_unaligned()` - Load from arbitrary address
3. `test_vec_store_aligned()` - Store to aligned buffer
4. `test_vec_store_unaligned()` - Store to arbitrary buffer
5. `test_vec_set1()` - Broadcast single byte to all lanes
6. `test_vec_zero()` - Create zero vector

**Implementation:**
- Define `vec128_t`, `vec256_t`, `vec512_t` types in `include/strider/simd/vector.h`
- Implement load/store wrappers for each architecture
- Create both scalar and SIMD variants
- Validate results match between implementations

#### TDD Cycle 4: Vector Comparison Operations
**Tests to write first:**
1. `test_vec_cmpeq_all_equal()` - Compare equal vectors
2. `test_vec_cmpeq_none_equal()` - Compare different vectors
3. `test_vec_cmpeq_partial()` - Some lanes match
4. `test_vec_to_mask()` - Extract comparison mask
5. `test_vec_mask_count_ones()` - Count matching lanes

**Implementation:**
- Implement `vec_cmpeq()` for byte comparison
- Implement `vec_to_mask()` for result extraction
- Create platform-specific implementations (AVX2/NEON)
- Validate mask bits match expected pattern

#### TDD Cycle 5: Memory Utilities
**Tests to write first:**
1. `test_aligned_alloc_16()` - 16-byte alignment
2. `test_aligned_alloc_32()` - 32-byte alignment
3. `test_aligned_alloc_64()` - 64-byte alignment
4. `test_aligned_free()` - Proper deallocation
5. `test_is_aligned()` - Alignment validation
6. `test_buffer_view_create()` - Zero-copy view

**Implementation:**
- Create `include/strider/utils/memory.h`
- Implement `strider_aligned_alloc()` / `strider_aligned_free()`
- Implement `strider_is_aligned()` checker
- Create buffer view structures

**Deliverables:**
- Working CMake build system with CTest
- Comprehensive test suite (>50 tests)
- SIMD abstraction headers validated by tests
- Feature detection with >90% coverage
- CI configuration (GitHub Actions)

---

### Phase 2: String Search Primitives (TDD)
**Goal:** Implement fundamental vectorized search operations
**Approach:** Test correctness against scalar reference, then optimize

#### TDD Cycle 6: Single Character Search
**Tests to write first (scalar reference):**
1. `test_strchr_found_first()` - Character at start
2. `test_strchr_found_middle()` - Character in middle
3. `test_strchr_found_last()` - Character at end
4. `test_strchr_not_found()` - Character not present
5. `test_strchr_empty_string()` - Empty input
6. `test_strchr_null_terminator()` - Search for '\0'
7. `test_strchr_multiple_occurrences()` - Return first match

**Tests for SIMD implementation:**
8. `test_strchr_simd_aligned()` - Aligned buffer
9. `test_strchr_simd_unaligned_prefix()` - Handle prefix
10. `test_strchr_simd_unaligned_suffix()` - Handle suffix
11. `test_strchr_simd_matches_scalar()` - Fuzz test equivalence
12. `test_strchr_simd_performance()` - Benchmark vs libc

**Implementation order:**
1. Write scalar reference implementation
2. Write all tests against scalar version
3. Implement SIMD version (TDD: one test at a time)
4. Validate SIMD matches scalar on 10000+ random inputs

#### TDD Cycle 7: Newline Detection
**Tests to write first:**
1. `test_find_newlines_unix()` - Count `\n` only
2. `test_find_newlines_windows()` - Count `\r\n` pairs
3. `test_find_newlines_mac_classic()` - Count `\r` only
4. `test_find_newlines_mixed()` - Handle all variants
5. `test_find_newlines_consecutive()` - Multiple newlines in a row
6. `test_find_newlines_none()` - No newlines present
7. `test_find_newlines_offsets()` - Return array of positions
8. `test_find_newlines_large_buffer()` - 1MB+ data

**SIMD-specific tests:**
9. `test_newlines_simd_vectorized()` - Process 32 bytes at once
10. `test_newlines_simd_unaligned()` - Handle any alignment
11. `test_newlines_simd_matches_wc()` - Match `wc -l` output

**Implementation:**
- Create `include/strider/parsers/newline.h`
- Scalar implementation first
- SIMD implementation with proper prefix/suffix handling
- Benchmark against `wc -l`

#### TDD Cycle 8: Multi-Character Pattern Search
**Tests to write first:**
1. `test_pattern_search_2byte()` - Short pattern
2. `test_pattern_search_4byte()` - Common case
3. `test_pattern_search_16byte()` - One vector width
4. `test_pattern_search_not_found()` - No matches
5. `test_pattern_search_overlapping()` - Overlapping occurrences
6. `test_pattern_search_boundary()` - Pattern crosses vector boundary

**Implementation:**
- Scalar implementation using naive search
- SIMD using two-character hashing for first pass
- Full validation on candidate positions

#### TDD Cycle 9: Whitespace Scanning
**Tests to write first:**
1. `test_skip_spaces_leading()` - Skip prefix spaces
2. `test_skip_spaces_trailing()` - Skip suffix spaces
3. `test_skip_spaces_mixed()` - Spaces and tabs
4. `test_count_whitespace_run()` - Count consecutive spaces
5. `test_trim_whitespace()` - Remove both ends

**Implementation:**
- Implement using character search primitives
- Optimize for common log formats

**Deliverables:**
- 4 primitive search functions with scalar + SIMD variants
- 50+ unit tests covering edge cases
- Property-based tests (fuzz testing)
- Performance benchmarks vs standard library
- Test coverage >95%

---

### Phase 3: Log Parsing Components (TDD)
**Goal:** Build higher-level log parsing functionality
**Approach:** Test with real-world log samples from day one

#### TDD Cycle 10: Timestamp Extraction
**Test data preparation:**
- Create `tests/data/` with sample log formats
- Include Apache, nginx, syslog, application logs
- Generate edge cases (invalid dates, partial timestamps)

**Tests to write first:**
1. `test_detect_iso8601()` - "2025-12-31T10:30:45Z"
2. `test_detect_rfc3339()` - "2025-12-31T10:30:45.123+00:00"
3. `test_detect_syslog()` - "Dec 31 10:30:45"
4. `test_detect_apache_common()` - "[31/Dec/2025:10:30:45 +0000]"
5. `test_extract_timestamp_bounds()` - Return start/end offsets
6. `test_timestamp_validation()` - Reject invalid dates
7. `test_timestamp_no_match()` - Return NULL if not found

**Implementation:**
- Start with regex-like state machine
- Optimize hot paths with SIMD
- Vectorize digit validation

#### TDD Cycle 11: Log Level Filtering
**Tests to write first:**
1. `test_match_level_error()` - Find "ERROR" lines
2. `test_match_level_case_insensitive()` - "error" = "ERROR"
3. `test_match_level_warn()` - "WARN", "WARNING"
4. `test_match_level_info()` - "INFO"
5. `test_match_level_debug()` - "DEBUG"
6. `test_match_multiple_levels()` - Bitmask for ERROR|WARN
7. `test_custom_level_strings()` - User-defined levels
8. `test_level_with_brackets()` - "[ERROR]" or "(ERROR)"

**Implementation:**
- Use pattern search primitives
- Return bitmask or offset array
- Support prefix matching

#### TDD Cycle 12: Field Tokenization
**Tests to write first:**
1. `test_split_by_space()` - "field1 field2 field3"
2. `test_split_by_comma()` - CSV parsing
3. `test_split_by_pipe()` - "field1|field2|field3"
4. `test_split_quoted_fields()` - "field1 \"field 2\" field3"
5. `test_split_escaped_quotes()` - Handle \" inside quotes
6. `test_split_empty_fields()` - "field1||field3"
7. `test_split_max_fields()` - Limit number of splits
8. `test_tokenize_zero_copy()` - Return views, not copies

**Implementation:**
- Create `strider_string_view_t` struct
- Use whitespace scanning primitives
- Handle quotes and escapes carefully

#### TDD Cycle 13: End-to-End Log Parsing
**Integration tests:**
1. `test_parse_apache_access_log()` - Full line parsing
2. `test_parse_nginx_error_log()` - Extract all fields
3. `test_parse_json_log()` - Structured logs
4. `test_filter_errors_from_file()` - Real file processing
5. `test_performance_vs_grep()` - Benchmark

**Implementation:**
- Combine primitives into high-level API
- Provide convenience functions
- Example: `strider_parse_log_line(format, line, &result)`

**Deliverables:**
- 3 major parsing components (timestamp, level, tokenize)
- 40+ tests with real log samples
- Test corpus in `tests/data/` with 100+ sample lines
- Integration tests for common log formats
- Performance comparison report

---

### Phase 4: Performance Optimization (TDD)
**Goal:** Maximize throughput while maintaining correctness
**Approach:** Performance tests are part of TDD - regressions fail the build

#### TDD Cycle 14: Performance Test Framework
**Tests to write first:**
1. `test_benchmark_strchr_throughput()` - Measure GB/s
2. `test_benchmark_newline_count()` - Compare vs `wc -l`
3. `test_benchmark_pattern_search()` - Compare vs `strstr`
4. `test_performance_regression()` - Fail if 10% slower
5. `test_benchmark_memory_bandwidth()` - Measure cache efficiency

**Implementation:**
- Integrate Google Benchmark or similar
- Create standard test datasets (1MB, 10MB, 100MB)
- Set baseline performance metrics
- Automate performance regression detection

#### TDD Cycle 15: Algorithm Tuning (Test-Driven)
**Performance tests guide optimization:**
1. Profile current implementation
2. Write test for expected speedup (e.g., "newline search should process >5 GB/s")
3. Optimize until test passes
4. Verify correctness tests still pass

**Optimizations to test:**
- Loop unrolling (test various factors: 2x, 4x, 8x)
- Branch elimination (measure branch mispredictions)
- Prefetching (test with/without on large datasets)
- Cache blocking (test various block sizes)

**Each optimization requires:**
- Before/after performance test
- Validation that correctness tests pass
- Documentation of hardware-specific behavior

#### TDD Cycle 16: Architecture-Specific Paths
**Tests for AVX-512:**
1. `test_avx512_available()` - Runtime detection
2. `test_avx512_newline_search()` - 64 bytes/iteration
3. `test_avx512_matches_avx2()` - Same results
4. `test_avx512_faster_than_avx2()` - Measure speedup

**Tests for ARM NEON:**
1. `test_neon_available()` - Runtime detection
2. `test_neon_newline_search()` - 16 bytes/iteration
3. `test_neon_matches_scalar()` - Correctness
4. `test_neon_faster_than_scalar()` - Measure speedup

**Implementation:**
- Separate code paths for each SIMD level
- Runtime dispatch based on CPU features
- Compile-time selection for embedded targets

#### TDD Cycle 17: Memory Access Optimization
**Tests to measure:**
1. `test_cache_miss_rate()` - Using perf counters
2. `test_memory_bandwidth_usage()` - Bytes/second
3. `test_streaming_vs_random()` - Access pattern impact
4. `test_large_dataset_scaling()` - Performance on 1GB+ files

**Implementation:**
- Use perf/VTune for profiling
- Optimize memory access patterns
- Validate with performance tests

**Deliverables:**
- Performance test suite (20+ benchmarks)
- Automated regression detection in CI
- Performance comparison report (Strider vs competitors)
- Optimization guide with benchmark results
- Architecture-specific tuning parameters

---

### Phase 5: API Refinement & Usability (TDD)
**Goal:** Create production-ready API with test-driven design
**Approach:** API usability validated through example programs

#### TDD Cycle 18: Public API Design
**Tests for API design:**
1. `test_api_naming_consistency()` - All functions use `strider_` prefix
2. `test_api_error_codes()` - Return value semantics
3. `test_api_null_safety()` - NULL pointer handling
4. `test_api_thread_safety()` - Concurrent usage
5. `test_api_const_correctness()` - Immutable inputs marked const

**Implementation:**
- Review all public headers
- Define `strider_error_t` enum
- Document every function with doxygen
- Create API stability guarantees

#### TDD Cycle 19: Error Handling
**Tests to write first:**
1. `test_error_invalid_input()` - NULL pointers
2. `test_error_buffer_too_small()` - Insufficient output buffer
3. `test_error_invalid_format()` - Bad format strings
4. `test_error_unsupported_platform()` - No SIMD available
5. `test_error_message_strings()` - Human-readable errors

**Implementation:**
- Define comprehensive error codes
- Add bounds checking (debug builds)
- Graceful degradation to scalar
- Error context with `strider_get_error_string()`

#### TDD Cycle 20: Example Programs (Executable Tests)
**Example programs as integration tests:**

1. **examples/01_hello_strider.c**
   - Test: Compiles and runs
   - Test: Counts newlines in README.md
   - Test: Output matches `wc -l`

2. **examples/02_log_filter.c**
   - Test: Filter ERROR logs from sample file
   - Test: Output contains only ERROR lines
   - Test: Performance >1M lines/sec

3. **examples/03_timestamp_extract.c**
   - Test: Extract timestamps from logs
   - Test: Parse 5 different timestamp formats
   - Test: Validate all timestamps correct

4. **examples/04_benchmark.c**
   - Test: Runs all benchmarks
   - Test: Reports results in machine-readable format
   - Test: Compares against baseline

**Each example must:**
- Include in CI testing
- Have expected output for validation
- Serve as API documentation
- Demonstrate best practices

#### TDD Cycle 21: Documentation Validation
**Documentation tests:**
1. `test_all_functions_documented()` - Doxygen coverage
2. `test_examples_compile()` - Code snippets valid
3. `test_api_examples_run()` - Example code produces output
4. `test_links_valid()` - No broken links in docs

**Documentation to create:**
- API reference (auto-generated from headers)
- Getting started guide
- Performance tuning guide
- Migration guide from libc functions
- FAQ

**Deliverables:**
- Stable public API (v0.1.0)
- 100% documented API surface
- 4+ example programs with tests
- Complete documentation suite
- API design rationale document

---

### Phase 6: Advanced Testing & Validation (TDD)
**Goal:** Comprehensive quality assurance
**Approach:** This phase runs continuously from Phase 1, not at the end

#### TDD Cycle 22: Fuzzing Infrastructure (Continuous)
**Fuzzing targets to create:**
1. `fuzz_strchr.c` - Fuzz single character search
   - Test: No crashes on random inputs
   - Test: Always matches scalar reference
   - Test: Handles all byte values (0-255)

2. `fuzz_newline.c` - Fuzz newline detection
   - Test: No crashes on random data
   - Test: Count matches `wc -l`
   - Test: Handles mixed line endings

3. `fuzz_pattern_search.c` - Fuzz pattern matching
   - Test: No crashes on random patterns
   - Test: Results match naive search
   - Test: Handles pattern longer than haystack

4. `fuzz_log_parser.c` - Fuzz complete parser
   - Test: No crashes on malformed logs
   - Test: Memory-safe on edge cases
   - Test: Validates against sanitizers

**Implementation:**
- Integrate libFuzzer or AFL++
- Run in CI for 10+ minutes per target
- Maintain corpus of interesting inputs
- Fix all crashes immediately (TDD: fuzz test becomes unit test)

#### TDD Cycle 23: Cross-Platform Validation
**CI test matrix:**
```yaml
platform:
  - ubuntu-latest (x86_64, AVX2)
  - ubuntu-latest (x86_64, AVX-512, emulated)
  - macos-latest (x86_64, AVX2)
  - macos-latest (ARM64, NEON, M1)
  - ubuntu-arm64 (ARM64, NEON)
compiler:
  - gcc-11, gcc-12, gcc-13
  - clang-14, clang-15, clang-16
build_type:
  - Debug (sanitizers enabled)
  - Release (optimizations enabled)
```

**Tests for each configuration:**
1. All unit tests pass
2. All integration tests pass
3. Performance tests meet minimums
4. No sanitizer warnings
5. Examples build and run

#### TDD Cycle 24: Property-Based Testing
**Properties to test:**
1. **Idempotence**: `find_newlines(s) == find_newlines(s)`
2. **Commutativity**: Results independent of buffer alignment
3. **Consistency**: SIMD matches scalar on all inputs
4. **Bounds**: Never read/write outside buffer
5. **Determinism**: Same input always produces same output

**Implementation using Hypothesis/QuickCheck approach:**
```c
test_property_newline_count_matches_strlen() {
    for (1000 iterations) {
        char* random_string = generate_random();
        assert(simd_count(random_string) == scalar_count(random_string));
    }
}
```

#### TDD Cycle 25: Coverage and Quality Metrics
**Coverage tests:**
1. `test_line_coverage()` - Require >95%
2. `test_branch_coverage()` - Require >90%
3. `test_function_coverage()` - Require 100% public API
4. `test_no_dead_code()` - No unreachable code

**Static analysis (part of CI):**
1. `clang-tidy` - No warnings
2. `cppcheck` - No issues
3. `valgrind` - No leaks
4. `sanitizers` - ASAN, UBSAN, MSAN clean

**Implementation:**
- gcov/lcov for coverage
- Upload to Codecov/Coveralls
- Block PR if coverage drops
- Daily static analysis reports

#### TDD Cycle 26: Regression Test Suite
**Regression tests track:**
1. Bug fixes (each bug becomes a test)
2. Performance baselines
3. API compatibility
4. Example program outputs

**Implementation:**
- Golden file testing for outputs
- Performance database (track over time)
- API compatibility checker
- Automated bisection on failures

**Deliverables:**
- 200+ total tests (unit + integration + fuzz)
- >95% code coverage
- CI/CD pipeline with 20+ configurations
- Fuzzing running continuously
- Nightly builds with full test suite
- Coverage and quality badges

---

## Technical Specifications

### Supported Platforms
- **x86_64:** Linux, macOS, Windows (AVX2 minimum, AVX-512 optional)
- **ARM64:** Linux, macOS (NEON standard, SVE future)
- **Compilers:** GCC 9+, Clang 10+, MSVC 2019+

### Performance Targets
- **Newline scanning:** >10 GB/s on modern CPUs
- **Pattern matching:** 5-8x faster than `strstr`
- **Log parsing:** Process 1M lines/sec on single core

### Dependencies
- **Runtime:** None (header-only C library)
- **Build:** CMake 3.15+
- **Testing:** Unity/Criterion/cmocka (unit tests)
- **Benchmarking:** Google Benchmark
- **Fuzzing:** libFuzzer or AFL++
- **Coverage:** gcov/lcov

### API Stability
- Semantic versioning
- Stable API after v1.0
- Deprecation warnings for breaking changes

---

## Risk Assessment

### Technical Risks
| Risk | Impact | Mitigation |
|------|--------|------------|
| Unaligned data performance | High | Provide aligned buffer utilities, handle unaligned prefixes |
| Portability issues | Medium | Extensive CI testing, fallback implementations |
| Complexity of SIMD code | Medium | Heavy documentation, unit tests for each variant |
| Performance variations | Low | Benchmark on target hardware, tuning parameters |

### Project Risks
| Risk | Impact | Mitigation |
|------|--------|------------|
| Scope creep | Medium | Focus on POC, defer advanced features |
| Insufficient testing | High | Automated CI, fuzzing, coverage tracking |
| Documentation lag | Medium | Write docs alongside code, examples first |

---

## Success Criteria

### POC Success
- [x] Build on x86_64 and ARM64
- [x] Demonstrate 5x+ speedup over scalar code
- [x] Parse real-world log formats correctly
- [x] Clean, usable API

### Production Readiness (Future)
- [ ] Stable API (v1.0)
- [ ] >95% test coverage
- [ ] Published benchmarks
- [ ] Adoption by at least one external project

---

## Timeline & Milestones (TDD Approach)

**Note:** Testing is continuous throughout, not a final phase. Test count and coverage increase with each cycle.

### Phase 1: Foundation (Week 1-2) - TDD Cycles 1-5
- Day 1-2: Build system with test framework
- Day 3-4: SIMD detection (10 tests)
- Day 5-7: Vector load/store (15 tests)
- Day 8-10: Vector comparison (12 tests)
- Day 11-14: Memory utilities (10 tests)
- **Milestone:** 50+ tests passing, CI running

### Phase 2: Primitives (Week 3-4) - TDD Cycles 6-9
- Day 15-18: Single char search (20 tests)
- Day 19-22: Newline detection (15 tests)
- Day 23-25: Pattern search (18 tests)
- Day 26-28: Whitespace scanning (10 tests)
- **Milestone:** 110+ tests, first benchmarks

### Phase 3: Log Parsing (Week 5-6) - TDD Cycles 10-13
- Day 29-32: Timestamp extraction (15 tests)
- Day 33-36: Log level filtering (12 tests)
- Day 37-39: Field tokenization (15 tests)
- Day 40-42: Integration tests (10 tests)
- **Milestone:** 160+ tests, test data corpus

### Phase 4: Optimization (Week 7-8) - TDD Cycles 14-17
- Day 43-45: Performance test framework
- Day 46-50: Algorithm tuning (guided by perf tests)
- Day 51-54: Architecture-specific optimizations
- Day 55-56: Memory access optimization
- **Milestone:** Performance tests in CI, baseline established

### Phase 5: API & Docs (Week 9-10) - TDD Cycles 18-21
- Day 57-60: API refinement (10 tests)
- Day 61-63: Error handling (8 tests)
- Day 64-66: Example programs (4 examples as tests)
- Day 67-70: Documentation with validation
- **Milestone:** API freeze, 100% documented

### Phase 6: Advanced Testing (Week 11-12) - TDD Cycles 22-26
- Day 71-74: Fuzzing infrastructure
- Day 75-77: Cross-platform CI matrix
- Day 78-80: Property-based tests
- Day 81-83: Coverage and static analysis
- Day 84: Final regression suite
- **Milestone:** 200+ tests, >95% coverage, POC complete

**Total Estimated Duration:** 12 weeks (3 months) for POC
**Test Count Trajectory:** 50 → 110 → 160 → 180 → 200+ tests

---

## Next Steps (TDD Start)

### Day 1 - Morning: Build System
1. **First Test to Write:**
   ```c
   // tests/test_build.c
   void test_build_system_works() {
       assert(1 == 1); // Simplest possible test
   }
   ```
2. Initialize CMake with CTest
3. Make the test pass
4. Commit: "Add build system with first passing test"

### Day 1 - Afternoon: SIMD Detection
1. **Write Failing Test:**
   ```c
   void test_cpu_has_sse2() {
       assert(strider_cpu_has_sse2() == true); // Will fail, not implemented
   }
   ```
2. Implement minimal code to pass
3. Refactor
4. Commit: "Add SSE2 detection"

### Day 2: Core Abstractions
1. Write test for `vec_load_aligned()`
2. Implement to pass test
3. Write test for `vec_store_aligned()`
4. Implement to pass test
5. Continue Red-Green-Refactor cycle

### Week 1 Goal
- Complete TDD Cycles 1-3
- Have 30+ passing tests
- CI running on GitHub Actions
- README with "Badges: Build Passing, Coverage 95%+"

### Community Engagement
- Open-source under MIT License
- GitHub repo with test results visible
- CONTRIBUTING.md emphasizing TDD workflow
- Require tests for all PRs

---

## References & Resources

### SIMD Programming Guides
- Intel Intrinsics Guide: https://www.intel.com/content/www/us/en/docs/intrinsics-guide/
- ARM NEON Intrinsics: https://developer.arm.com/architectures/instruction-sets/intrinsics/
- Agner Fog's Optimization Manuals: https://www.agner.org/optimize/

### Related Projects
- simdjson: SIMD JSON parsing
- hyperscan: SIMD regex matching
- rapidstring: Fast string library
- yyjson: High-performance JSON library

### Benchmarking Tools
- Google Benchmark
- perf (Linux)
- Instruments (macOS)
- VTune (Intel)

### TDD Resources
- "Test Driven Development: By Example" by Kent Beck
- "Modern C Testing" practices
- Unity Test Framework documentation
- Property-based testing in C

---

## TDD Success Metrics

Track these throughout development:

| Metric | Target | Purpose |
|--------|--------|---------|
| Test count | 200+ | Comprehensive coverage |
| Code coverage | >95% | All paths tested |
| Tests written first | >90% | True TDD practice |
| CI test time | <5 min | Fast feedback |
| Bugs escaped to production | 0 | Quality assurance |
| Performance regressions | 0 | Continuous validation |

---

**Document Version:** 2.0 (TDD Revision)
**Last Updated:** 2025-12-31
**Status:** Initial Planning - TDD Approach
**Methodology:** Test-Driven Development (Red-Green-Refactor)
