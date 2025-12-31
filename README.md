# Strider ⚡

High-performance string log processing via SIMD abstraction.

## Project Status

🚧 **Under Development** - TDD Phase 1: Foundation

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![Tests](https://img.shields.io/badge/tests-41%2F41%20passing-brightgreen)]()
[![Coverage](https://img.shields.io/badge/coverage-100%25-brightgreen)]()

## Overview

Strider is a Proof of Concept (POC) demonstrating how to accelerate common log-parsing tasks using hardware-level parallelism (SIMD instructions).

### Key Features

- **Multi-Arch Support**: Unified interface for x86_64 (AVX2/AVX-512) and ARM64 (NEON)
- **Vectorized Search**: Uses SIMD instructions to scan megabytes of log data in a single pass
- **Zero-Copy**: Designed for data-oriented workflows to minimize memory overhead
- **Header-Only**: Minimal runtime overhead with compile-time optimization
- **Test-Driven**: Every feature developed using Red-Green-Refactor cycle

## Performance Targets

- **Newline scanning:** >10 GB/s on modern CPUs
- **Pattern matching:** 5-8x faster than `strstr`
- **Log parsing:** Process 1M lines/sec on single core

## Building

### Prerequisites

- CMake 3.15+
- GCC 9+ or Clang 10+
- AVX2 support (x86_64) or NEON (ARM64)

### Build Steps

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Running Tests

```bash
cd build
ctest --output-on-failure
```

## Development Approach

This project follows **Test-Driven Development (TDD)**:

1. **RED**: Write a failing test
2. **GREEN**: Write minimal code to pass
3. **REFACTOR**: Improve while keeping tests green

See [DEVELOPMENT_PLAN.md](docs/plans/DEVELOPMENT_PLAN.md) for the complete 12-week roadmap.

## Current Progress

### Phase 1: Foundation & Core Infrastructure (Week 1-2)

- [x] TDD Cycle 1: Build System & Test Framework
  - [x] CMake with CTest integration
  - [x] Unity test framework
  - [x] Platform detection
  - [x] First 3 tests passing
- [x] TDD Cycle 2: SIMD Feature Detection
  - [x] Runtime CPU feature detection (CPUID/hwcaps)
  - [x] SSE2, AVX2, AVX-512 detection (x86_64)
  - [x] NEON detection (ARM64)
  - [x] Compile-time and runtime consistency
  - [x] 7 additional tests passing (10 total)
- [x] TDD Cycle 3: Core Vector Abstraction - Load/Store
  - [x] 128-bit vector types (SSE2/NEON)
  - [x] 256-bit vector types (AVX2)
  - [x] Aligned and unaligned load/store
  - [x] Set1 (broadcast) and zero operations
  - [x] Alignment helper utilities
  - [x] 10 additional tests passing (20 total)
- [x] TDD Cycle 4: Vector Comparison Operations
  - [x] Element-wise equality comparison (cmpeq)
  - [x] Extract bitmask from comparison results (movemask)
  - [x] Byte search operations (strchr-like)
  - [x] Bit manipulation utilities (ctz, popcount)
  - [x] Both 128-bit and 256-bit variants
  - [x] 9 additional tests passing (29 total)
- [x] TDD Cycle 5: Memory Utilities
  - [x] Aligned memory allocation (16/32/64-byte)
  - [x] Alignment checking utilities
  - [x] Buffer view abstraction (zero-copy)
  - [x] Buffer slicing and manipulation
  - [x] Cross-platform support (C11/POSIX/MSVC)
  - [x] 12 additional tests passing (41 total)

## Project Structure

```
strider/
├── include/strider/       # Public header files (header-only library)
├── tests/                 # Unit tests and test framework
│   ├── unity/            # Unity test framework
│   └── test_build.c      # Build system tests
├── examples/             # Example programs
├── docs/                 # Documentation
│   └── plans/           # Development planning documents
├── CMakeLists.txt       # Main build configuration
└── README.md            # This file
```

## License

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

### Third-Party Licenses

This project uses the following open-source software:

- **Unity Test Framework** (MIT License) - Testing only, not distributed in library
  - Copyright (c) 2007-25 Mike Karlesky, Mark VanderVoord, & Greg Williams
  - See [tests/unity/unity.h](tests/unity/unity.h) for full license text

See [NOTICE](NOTICE) for complete attribution information.

## Contributing

This is a POC project. Contributions welcome once Phase 1 is complete.

## References

- [Development Plan](docs/plans/DEVELOPMENT_PLAN.md)
- [Intel Intrinsics Guide](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/)
- [ARM NEON Intrinsics](https://developer.arm.com/architectures/instruction-sets/intrinsics/)
