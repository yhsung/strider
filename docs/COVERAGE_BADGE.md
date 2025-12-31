# Setting Up Dynamic Coverage Badge

This document explains how to set up a dynamic coverage badge for the Strider project. The workflow is **already configured** - you just need to set up the badge endpoint!

## Quick Start (Recommended)

Follow these steps to get a dynamic coverage badge in **under 5 minutes**:

### Step 1: Create a GitHub Gist

1. Go to https://gist.github.com
2. Click "Create new gist"
3. Set filename: `coverage-badge.json`
4. Paste this content:
   ```json
   {
     "schemaVersion": 1,
     "label": "coverage",
     "message": "100%",
     "color": "brightgreen"
   }
   ```
5. Click "Create public gist"
6. **Copy the Gist ID** from the URL (e.g., `https://gist.github.com/username/abc123def456` → `abc123def456`)

### Step 2: Create GitHub Personal Access Token

1. Go to https://github.com/settings/tokens
2. Click "Generate new token (classic)"
3. Give it a name: `Strider Coverage Badge`
4. Select scope: **only check `gist`** (write access to gists)
5. Click "Generate token"
6. **Copy the token** (you won't see it again!)

### Step 3: Add Secrets to Repository

1. Go to your repository: https://github.com/yhsung/strider
2. Click **Settings** → **Secrets and variables** → **Actions**
3. Click **New repository secret** twice to add:
   - Name: `GIST_TOKEN`, Value: (paste your token from step 2)
   - Name: `GIST_ID`, Value: (paste your gist ID from step 1)

### Step 4: Update README Badge

Replace the current coverage badge line in `README.md`:

**Current (static):**
```markdown
[![Coverage](https://img.shields.io/badge/coverage-100%25-brightgreen)]()
```

**New (dynamic):**
```markdown
[![Coverage](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/yhsung/YOUR_GIST_ID/raw/coverage-badge.json)](https://github.com/yhsung/strider/actions/workflows/ci.yml)
```

Replace `YOUR_GIST_ID` with your actual Gist ID from step 1.

### Step 5: Push and Verify

1. Commit the README change
2. Push to `main` branch
3. Wait for CI to complete
4. Refresh your repository page - the badge should now be dynamic! 🎉

The badge will automatically update on every push to `main` with:
- ✅ Current coverage percentage
- ✅ Color coding (green ≥90%, yellow ≥60%, red <60%)

---

## Alternative: Codecov Badge

If you prefer a full-featured coverage service with reports and graphs:

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

---

## How It Works

The CI workflow automatically:

1. **Runs tests** with coverage instrumentation
2. **Generates coverage report** using lcov
3. **Extracts coverage percentage** from the report
4. **Creates badge JSON** with appropriate color:
   - 🟢 Green (≥90%)
   - 🟢 Light green (≥75%)
   - 🟡 Yellow (≥60%)
   - 🔴 Red (<60%)
5. **Updates your Gist** with the new coverage data (on pushes to `main`)
6. **shields.io reads the Gist** and displays it as a badge

## Troubleshooting

### Badge not updating?

1. **Check secrets are set correctly**
   - Go to Repository Settings → Secrets and variables → Actions
   - Verify both `GIST_TOKEN` and `GIST_ID` exist

2. **Check CI workflow completed**
   - Go to Actions tab in your repository
   - Ensure the "Coverage Report" job succeeded
   - Look for the "Update coverage badge gist" step

3. **Check Gist was updated**
   - Visit your Gist URL
   - Verify it shows the latest coverage percentage

4. **Clear badge cache**
   - shields.io caches badges for performance
   - Add `?cache=300` to your badge URL to reduce cache time:
   ```markdown
   [![Coverage](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/yhsung/GIST_ID/raw/coverage-badge.json&cache=300)](https://github.com/yhsung/strider/actions/workflows/ci.yml)
   ```

### Permission denied when updating Gist?

- Ensure your Personal Access Token has the `gist` scope
- Regenerate the token if needed
- Update the `GIST_TOKEN` secret

## View Coverage Locally

Generate coverage reports on your development machine:

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

## Summary

The **Quick Start** approach (Gist + shields.io) gives you:
- ✅ **5-minute setup** - Just create a Gist and add 2 secrets
- ✅ **Automatic updates** - Badge updates on every push to `main`
- ✅ **No external service** - Uses only GitHub infrastructure
- ✅ **Color coding** - Visual indication of coverage health
- ✅ **Already configured** - Workflow is ready to go!

**Alternative:** Codecov provides richer features (trends, PR comments, line-by-line coverage) if you need detailed analysis.

Both options are integrated into the CI workflow and work automatically once configured!
