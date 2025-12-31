#!/bin/bash
# Format all C/H files with clang-format
# Usage: ./scripts/format-code.sh [--check]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    echo "Error: clang-format is not installed"
    echo "Install with: sudo apt-get install clang-format"
    exit 1
fi

# Find all C/H files (excluding Unity framework)
FILES=$(find "$PROJECT_ROOT" \
    -path "$PROJECT_ROOT/build" -prune -o \
    -path "$PROJECT_ROOT/tests/unity" -prune -o \
    -type f \( -name "*.c" -o -name "*.h" \) -print)

if [ "$1" = "--check" ]; then
    echo "Checking code formatting..."
    FAILED=0
    for file in $FILES; do
        if ! clang-format --dry-run --Werror "$file" 2>/dev/null; then
            echo "❌ $file needs formatting"
            FAILED=1
        fi
    done

    if [ $FAILED -eq 1 ]; then
        echo ""
        echo "Format check failed. Run './scripts/format-code.sh' to fix formatting."
        exit 1
    else
        echo "✅ All files are properly formatted!"
        exit 0
    fi
else
    echo "Formatting C/H files..."
    for file in $FILES; do
        echo "Formatting: $file"
        clang-format -i "$file"
    done
    echo "✅ Done! All files formatted."
fi
