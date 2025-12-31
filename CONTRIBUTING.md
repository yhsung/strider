# Contributing to Strider

Thank you for your interest in contributing to Strider! This document provides guidelines for contributing to the project.

## Development Philosophy

Strider follows **Test-Driven Development (TDD)**:
1. **RED**: Write a failing test
2. **GREEN**: Write minimal code to make it pass
3. **REFACTOR**: Improve while keeping tests green

## Before You Start

1. Fork the repository
2. Create a feature branch from `develop`
3. Read the [Development Plan](docs/plans/DEVELOPMENT_PLAN.md)

## Development Workflow

### 1. Writing Code

```bash
# Clone your fork
git clone https://github.com/YOUR_USERNAME/strider-dev.git
cd strider-dev

# Create feature branch
git checkout -b feature/your-feature-name

# Build and test locally
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DSTRIDER_BUILD_TESTS=ON
cmake --build .
ctest --output-on-failure
```

### 2. Testing Requirements

**All contributions must:**
- Include unit tests (TDD approach)
- Pass all existing tests
- Maintain 100% test pass rate
- Work on Linux, macOS, and Windows
- Pass sanitizer checks (ASan, UBSan)

**Run tests locally:**
```bash
# Standard tests
ctest --output-on-failure

# With AddressSanitizer
cmake .. -DSTRIDER_ENABLE_ASAN=ON
cmake --build .
ctest --output-on-failure

# With UndefinedBehaviorSanitizer
cmake .. -DSTRIDER_ENABLE_UBSAN=ON
cmake --build .
ctest --output-on-failure
```

### 3. Code Style

- **C Standard**: C11
- **Indentation**: 4 spaces (no tabs in source files)
- **Line length**: 100 characters recommended
- **Naming**:
  - Functions: `strider_snake_case()`
  - Types: `strider_type_name_t`
  - Macros: `STRIDER_UPPER_CASE`
- **Comments**: Use `/* */` for multi-line, `//` for single-line
- **Documentation**: Every public API must have Doxygen comments

### 4. Commit Messages

Follow conventional commits format:

```
<type>: <description>

[optional body]

[optional footer]
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `test`: Adding or updating tests
- `docs`: Documentation changes
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `ci`: CI/CD changes

**Examples:**
```
feat: Add AVX-512 support for pattern matching

Implements vectorized pattern search using AVX-512 instructions
for 512-bit wide operations. Includes fallback to AVX2.

Closes #42
```

### 5. Pull Request Process

1. **Update tests**: Add tests for new features
2. **Update docs**: Update README.md and code comments
3. **Run CI locally**: Ensure all tests pass
4. **Create PR**:
   - Target the `develop` branch
   - Reference related issues
   - Describe what changed and why
5. **CI checks**: GitHub Actions will run automatically
   - All platforms must pass
   - Coverage must not decrease
6. **Code review**: Wait for maintainer review
7. **Merge**: Squash and merge when approved

## CI/CD Pipeline

GitHub Actions runs automatically on every PR:

### Test Matrix
- **Linux**:
  - GCC (Debug, Release)
  - Clang (Debug, Release)
  - AddressSanitizer
  - UndefinedBehaviorSanitizer
- **macOS**:
  - ARM64 (Apple Silicon)
  - x86_64 (Intel)
- **Windows**:
  - MSVC (Debug, Release)

### Coverage
- Code coverage is tracked via Codecov
- New code should maintain >90% coverage
- Tests are excluded from coverage metrics

## Project Structure

```
strider/
├── .github/
│   └── workflows/        # CI/CD pipelines
├── include/strider/      # Public headers (header-only API)
│   ├── config.h         # Platform/CPU detection
│   ├── simd/            # SIMD abstractions
│   └── utils/           # Utilities (memory, etc.)
├── src/                 # Implementation files
│   └── config.c         # Runtime CPU detection
├── tests/               # Unit tests
│   ├── unity/          # Unity test framework
│   └── test_*.c        # Test files
├── examples/           # Example programs
├── docs/               # Documentation
│   └── plans/         # Development plans
└── CMakeLists.txt     # Build configuration
```

## License

By contributing, you agree that your contributions will be licensed under the Apache License 2.0.

## Questions?

- Open an issue for bugs or feature requests
- Check existing issues before creating new ones
- Be respectful and constructive in discussions

## Development Phases

We're currently in **Phase 1: Foundation** (Complete ✅)

See [DEVELOPMENT_PLAN.md](docs/plans/DEVELOPMENT_PLAN.md) for the full roadmap.

---

Thank you for contributing to Strider! 🚀
