#!/bin/bash
# Shell Script Linting for CARLA
# Usage: ./lint-shell.sh [file|directory]

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
SHELLCHECK="shellcheck"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}CARLA Shell Script Linter${NC}"
echo "=========================="

# Check if shellcheck is available
if ! command -v "$SHELLCHECK" &> /dev/null; then
    echo -e "${RED}Error: shellcheck not found${NC}"
    echo "Install with: brew install shellcheck (macOS) or sudo apt-get install shellcheck (Linux)"
    exit 1
fi

# Find shell scripts
if [ $# -eq 0 ]; then
    echo "Scanning for shell scripts..."
    FILES=$(find . -name "*.sh" -type f 2>/dev/null | grep -v ".git" | grep -v "node_modules")
else
    if [ -d "$1" ]; then
        FILES=$(find "$1" -name "*.sh" -type f 2>/dev/null)
    else
        FILES="$1"
    fi
fi

echo "Files to lint:"
echo "$FILES" | head -10
TOTAL=$(echo "$FILES" | wc -l)
echo "Total: $TOTAL files"
echo ""

ERRORS=0
WARNINGS=0

for file in $FILES; do
    if [ ! -f "$file" ]; then
        continue
    fi

    # Run shellcheck
    OUTPUT=$("$SHELLCHECK" "$file" 2>&1) || true

    FILE_ERRORS=$(echo "$OUTPUT" | grep -c "error:" || true)
    FILE_WARNINGS=$(echo "$OUTPUT" | grep -c "warning:" || true)

    if [ "$FILE_ERRORS" -gt 0 ] || [ "$FILE_WARNINGS" -gt 0 ]; then
        echo -e "${YELLOW}$file${NC}"
        echo "$OUTPUT" | grep -E "(error:|warning:)" | head -5
        echo ""
        ERRORS=$((ERRORS + FILE_ERRORS))
        WARNINGS=$((WARNINGS + FILE_WARNINGS))
    fi
done

echo "=========================="
echo -e "${GREEN}Summary:${NC}"
echo "  Errors:   $ERRORS"
echo "  Warnings: $WARNINGS"

if [ "$ERRORS" -gt 0 ]; then
    echo -e "${RED}Linting failed with $ERRORS errors${NC}"
    exit 1
else
    echo -e "${GREEN}Linting passed!${NC}"
    exit 0
fi
