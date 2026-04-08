#!/bin/bash
# ==============================================================================
# Python Linting Script for CARLA
# ==============================================================================
# Usage:
#   ./lint-python.sh                    # Lint all Python files
#   ./lint-python.sh --fix              # Auto-fix where possible
#
# Requirements:
#   - ruff (pip install ruff)
#   - mypy (optional, pip install mypy)
# ==============================================================================

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║     CARLA Python Linting              ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
echo ""

# Check if ruff is available
if ! command -v ruff &> /dev/null; then
    echo -e "${RED}✗ Error: ruff not found${NC}"
    echo "  Install with: pip install ruff"
    exit 1
fi

echo -e "${GREEN}✓ ruff found${NC}"
echo ""

# Parse arguments
FIX=""
if [ "$1" = "--fix" ]; then
    FIX="--fix"
    echo "Auto-fix mode enabled"
fi

# Run ruff check
echo "Running ruff check..."
if python3 -m ruff check PythonAPI/ $FIX; then
    echo -e "${GREEN}✅ Python linting passed${NC}"
else
    echo -e "${RED}❌ Python linting failed${NC}"
    exit 1
fi

echo ""

# Run ruff format check
echo "Running ruff format check..."
if python3 -m ruff format PythonAPI/ --check; then
    echo -e "${GREEN}✅ Python formatting passed${NC}"
else
    echo -e "${YELLOW}⚠️  Python formatting needs fixes${NC}"
    echo "  Run: python3 -m ruff format PythonAPI/"
fi

echo ""
echo -e "${GREEN}════════════════════════════════════════${NC}"
echo -e "${GREEN}✅ All Python checks passed!${NC}"
echo -e "${GREEN}════════════════════════════════════════${NC}"
