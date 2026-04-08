#!/bin/bash
# ==============================================================================
# YAML Linting for CARLA
# ==============================================================================
# Usage:
#   ./lint-yaml.sh                    # Lint all YAML files
#   ./lint-yaml.sh path/to/file.yml   # Lint specific file
#
# Requirements:
#   - yamllint (pip install yamllint)
# ==============================================================================

set -e

YAMLLINT="python3 -m yamllint"
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║        CARLA YAML Linter               ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
echo ""

# Check if yamllint is available
if ! command -v python3 &> /dev/null; then
    echo -e "${RED}✗ Error: python3 not found${NC}"
    exit 1
fi

if ! python3 -c "import yamllint" 2>/dev/null; then
    echo -e "${RED}✗ Error: yamllint not installed${NC}"
    echo "  Install with: pip install yamllint"
    exit 1
fi

echo -e "${GREEN}✓ yamllint found${NC}"

# Find YAML files
if [ $# -eq 0 ]; then
    echo "Scanning for YAML files..."
    FILES=$(find . -name "*.yaml" -o -name "*.yml" 2>/dev/null | grep -v ".git" | grep -v "node_modules")
else
    if [ -d "$1" ]; then
        FILES=$(find "$1" -name "*.yaml" -o -name "*.yml" 2>/dev/null)
    else
        FILES="$1"
    fi
fi

if [ -z "$FILES" ]; then
    echo -e "${YELLOW}No YAML files found${NC}"
    exit 0
fi

TOTAL=$(echo "$FILES" | wc -l)
echo -e "Files to lint: ${BLUE}$TOTAL${NC}"
echo ""

# Run yamllint
if $YAMLLINT -c "$PROJECT_ROOT/.yamllint" $FILES; then
    echo -e "${GREEN}✓ YAML linting passed!${NC}"
    exit 0
else
    echo -e "${RED}✗ YAML linting failed${NC}"
    exit 1
fi
