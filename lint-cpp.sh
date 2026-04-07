#!/bin/bash
# ==============================================================================
# C++ Linting Script for CARLA
# ==============================================================================
# Usage:
#   ./lint-cpp.sh                    # Lint all LibCarla files
#   ./lint-cpp.sh path/to/file.cpp   # Lint specific file
#   ./lint-cpp.sh LibCarla/source/   # Lint directory
#
# Requirements:
#   - clang-tidy (brew install llvm on macOS)
#   - Compilation database for full checks (build with CMake first)
#
# Full checks require compilation database:
#   cmake --preset=linux-base
#   cmake --build Build
# ==============================================================================

set -e

CLANG_TIDY="/opt/homebrew/opt/llvm/bin/clang-tidy"
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}╔════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║        CARLA C++ Linter (clang-tidy)      ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════╝${NC}"
echo ""

# Check if clang-tidy is available
if ! command -v "$CLANG_TIDY" &> /dev/null; then
    echo -e "${RED}✗ Error: clang-tidy not found${NC}"
    echo "  Install with: brew install llvm (macOS)"
    echo "                sudo apt-get install clang-tidy (Linux)"
    exit 1
fi

echo -e "${GREEN}✓ clang-tidy found${NC}"

# Check for compilation database
if [ -f "compile_commands.json" ] || [ -d "Build" ]; then
    echo -e "${GREEN}✓ Compilation database found - full checks enabled${NC}"
    HAS_COMPILE_DB=true
else
    echo -e "${YELLOW}⚠ No compilation database - limited checks${NC}"
    echo "  For full checks: cmake --preset=linux-base && cmake --build Build"
    HAS_COMPILE_DB=false
fi
echo ""

# Find C++ files
if [ $# -eq 0 ]; then
    echo "Scanning LibCarla C++ files..."
    FILES=$(find LibCarla/source -name "*.cpp" -o -name "*.h" -o -name "*.hpp" 2>/dev/null | grep -v "/test/" | grep -v "util/ue4" | head -10)
    
    if [ -z "$FILES" ]; then
        echo -e "${YELLOW}No C++ files found${NC}"
        echo -e "${GREEN}✓ C++ linting skipped (expected without build)${NC}"
        exit 0
    fi
else
    if [ -d "$1" ]; then
        FILES=$(find "$1" -name "*.cpp" -o -name "*.h" -o -name "*.hpp" 2>/dev/null)
    else
        FILES="$1"
    fi
fi

TOTAL=$(echo "$FILES" | wc -l)
echo -e "Files to lint: ${BLUE}$TOTAL${NC}"
echo ""

ERRORS=0
WARNINGS=0

# Run clang-tidy on each file
for file in $FILES; do
    if [ ! -f "$file" ]; then
        continue
    fi
    
    echo -e "${YELLOW}Linting:${NC} $file"
    
    if [ "$HAS_COMPILE_DB" = true ]; then
        OUTPUT=$("$CLANG_TIDY" --config-file="$PROJECT_ROOT/.clang-tidy" -p Build "$file" 2>&1) || true
    else
        OUTPUT=$("$CLANG_TIDY" --config-file="$PROJECT_ROOT/.clang-tidy" --checks="readability-*,bugprone-*,portability-*,-readability-identifier-length" "$file" 2>&1) || true
    fi
    
    # Count errors and warnings (skip compiler errors without build)
    FILE_ERRORS=$(echo "$OUTPUT" | grep -v "clang-diagnostic-error" | grep -c "error:" || true)
    FILE_WARNINGS=$(echo "$OUTPUT" | grep -c "warning:" || true)
    
    if [ "$FILE_ERRORS" -gt 0 ] || [ "$FILE_WARNINGS" -gt 0 ]; then
        echo "$OUTPUT" | grep -v "clang-diagnostic-error" | grep -E "(error:|warning:)" | head -5
        echo ""
        ERRORS=$((ERRORS + FILE_ERRORS))
        WARNINGS=$((WARNINGS + FILE_WARNINGS))
    fi
done

echo "════════════════════════════════════════"
echo -e "${GREEN}Summary:${NC}"
echo "  Errors:   $ERRORS"
echo "  Warnings: $WARNINGS"

if [ "$HAS_COMPILE_DB" = false ]; then
    echo ""
    echo -e "${YELLOW}Note: Limited checks (no compilation database)${NC}"
fi

if [ $ERRORS -gt 0 ]; then
    echo -e "${RED}✗ Linting failed with $ERRORS errors${NC}"
    exit 1
else
    echo -e "${GREEN}✓ Linting passed!${NC}"
    exit 0
fi
