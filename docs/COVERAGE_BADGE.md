# Setting Up Coverage Badge

This document explains how to set up a dynamic coverage badge for the Strider project.

## Option 1: Codecov Badge (Recommended)

Codecov provides automatic coverage badges and detailed reports.

### Setup Steps:

1. **Sign up for Codecov**
   - Go to [codecov.io](https://codecov.io)
   - Sign in with your GitHub account
   - Authorize Codecov to access your repository

2. **Add Repository**
   - Navigate to your repository on Codecov
   - Copy the badge markdown from the repository settings

3. **Update README**
   - Replace the current coverage badge in README.md with:
   ```markdown
   [![codecov](https://codecov.io/gh/yhsung/strider/branch/main/graph/badge.svg)](https://codecov.io/gh/yhsung/strider)
   ```

4. **Add Token (Optional for Public Repos)**
   - For private repos, add `CODECOV_TOKEN` to GitHub Secrets
   - Go to: Repository Settings → Secrets and variables → Actions → New repository secret
   - Name: `CODECOV_TOKEN`
   - Value: Copy from Codecov dashboard

### What You Get:
- ✅ Automatic badge updates on every CI run
- ✅ Detailed coverage reports with line-by-line breakdown
- ✅ Coverage trends over time
- ✅ PR comments with coverage changes

## Option 2: Shields.io Dynamic Badge

Use shields.io with endpoint functionality for a self-hosted badge.

### Setup Steps:

1. **Create a GitHub Gist**
   - Go to [gist.github.com](https://gist.github.com)
   - Create a new gist named `strider-coverage.json` with content:
   ```json
   {
     "schemaVersion": 1,
     "label": "coverage",
     "message": "100%",
     "color": "brightgreen"
   }
   ```
   - Make it public
   - Copy the Gist ID from the URL

2. **Create GitHub Token**
   - Go to GitHub Settings → Developer settings → Personal access tokens → Tokens (classic)
   - Generate new token with `gist` scope
   - Copy the token

3. **Add Secrets to Repository**
   - Go to: Repository Settings → Secrets and variables → Actions
   - Add two secrets:
     - `GIST_SECRET`: Your GitHub personal access token
     - `GIST_ID`: Your Gist ID

4. **Uncomment Badge Action**
   - In `.github/workflows/ci.yml`, the badge action is already configured
   - It will automatically update the gist with coverage percentage

5. **Update README Badge**
   ```markdown
   [![Coverage](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/USERNAME/GIST_ID/raw/strider-coverage.json)](https://github.com/yhsung/strider/actions/workflows/ci.yml)
   ```
   - Replace `USERNAME` with your GitHub username
   - Replace `GIST_ID` with your gist ID

### What You Get:
- ✅ Dynamic badge that updates on CI runs
- ✅ No external service dependency (besides GitHub)
- ✅ Full control over badge appearance

## Option 3: Static Badge (Current)

The current approach uses a static badge that must be manually updated.

**Pros:**
- ✅ No setup required
- ✅ Works immediately

**Cons:**
- ❌ Must be manually updated when coverage changes
- ❌ No historical tracking

## Current Coverage Reporting

The CI workflow already:
- ✅ Generates coverage reports using lcov
- ✅ Excludes test files and system headers
- ✅ Displays coverage summary in GitHub Actions output
- ✅ Uploads to Codecov (if configured)
- ✅ Shows coverage in GitHub Actions summary

### View Coverage Locally:

```bash
# Build with coverage
cmake -B build -DCMAKE_BUILD_TYPE=Debug \
  -DSTRIDER_BUILD_TESTS=ON \
  -DCMAKE_C_FLAGS="--coverage -fprofile-arcs -ftest-coverage"
cmake --build build
ctest --test-dir build

# Generate coverage report
lcov --capture --directory build --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.info
lcov --list coverage.info

# Generate HTML report
genhtml coverage.info --output-directory coverage_html
# Open coverage_html/index.html in browser
```

## Recommended Approach

**For open-source projects:** Use **Option 1 (Codecov)** for the best experience with minimal setup.

**For private projects or maximum control:** Use **Option 2 (Shields.io with Gist)**.

Both options provide automatic updates and are already integrated into the CI workflow!
