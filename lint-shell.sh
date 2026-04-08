#!/bin/bash
# ==============================================================================
# Shell Script Linting for CARLA
# ==============================================================================
# Usage:
#   ./lint-shell.sh                    # Lint all shell scripts
#   ./lint-shell.sh path/to/script.sh  # Lint specific script
#
# Requirements:
#   - shellcheck (brew install shellcheck on macOS)
# ==============================================================================

set -e

SHELLCHECK="shellcheck"
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║        CARLA Shell Linter              ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
echo ""

# Check if shellcheck is available
if ! command -v "$SHELLCHECK" &> /dev/null; then
    echo -e "${RED}✗ Error: shellcheck not found${NC}"
    echo "  Install with: brew install shellcheck (macOS)"
    echo "                sudo apt-get install shellcheck (Linux)"
    exit 1
fi

echo -e "${GREEN}✓ shellcheck found${NC}"

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

if [ -z "$FILES" ]; then
    echo -e "${YELLOW}No shell scripts found${NC}"
    exit 0
fi

TOTAL=$(echo "$FILES" | wc -l)
echo -e "Files to lint: ${BLUE}$TOTAL${NC}"
echo ""

ERRORS=0
WARNINGS=0

# Run shellcheck on each file
for file in $FILES; do
    if [ ! -f "$file" ]; then
        continue
    fi
    
    echo -e "${YELLOW}Linting:${NC} $file"
    
    OUTPUT=$("$SHELLCHECK" "$file" 2>&1) || true
    
    FILE_ERRORS=$(echo "$OUTPUT" | grep -c "error:" || true)
    FILE_WARNINGS=$(echo "$OUTPUT" | grep -c "warning:" || true)
    
    if [ "$FILE_ERRORS" -gt 0 ] || [ "$FILE_WARNINGS" -gt 0 ]; then
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

if [ $ERRORS -gt 0 ]; then
    echo -e "${RED}✗ Linting failed with $ERRORS errors${NC}"
    exit 1
else
    echo -e "${GREEN}✓ Linting passed!${NC}"
    exit 0
fi
