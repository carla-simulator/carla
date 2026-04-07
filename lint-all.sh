#!/bin/bash
# Comprehensive Linting Script for CARLA
# Usage: ./lint-all.sh [options]
# Options:
#   --python    Run Python linting only
#   --cpp       Run C++ linting only
#   --shell     Run shell linting only
#   --yaml      Run YAML linting only
#   --fix       Auto-fix where possible

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

PYTHON_ONLY=false
CPP_ONLY=false
SHELL_ONLY=false
YAML_ONLY=false
AUTO_FIX=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --python) PYTHON_ONLY=true; shift ;;
        --cpp) CPP_ONLY=true; shift ;;
        --shell) SHELL_ONLY=true; shift ;;
        --yaml) YAML_ONLY=true; shift ;;
        --fix) AUTO_FIX=true; shift ;;
        --help)
            echo "Usage: ./lint-all.sh [options]"
            echo "Options:"
            echo "  --python    Run Python linting only"
            echo "  --cpp       Run C++ linting only"
            echo "  --shell     Run shell linting only"
            echo "  --yaml      Run YAML linting only"
            echo "  --fix       Auto-fix where possible"
            echo "  --help      Show this help"
            exit 0
            ;;
        *) echo -e "${RED}Unknown option: $1${NC}"; exit 1 ;;
    esac
done

# If no specific option, run all
if [ "$PYTHON_ONLY" = false ] && [ "$CPP_ONLY" = false ] && [ "$SHELL_ONLY" = false ] && [ "$YAML_ONLY" = false ]; then
    PYTHON_ONLY=true
    CPP_ONLY=true
    SHELL_ONLY=true
    YAML_ONLY=true
fi

echo -e "${BLUE}╔════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║          CARLA Comprehensive Linting                  ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════╝${NC}"
echo ""

ERRORS=0

# Python Linting
if [ "$PYTHON_ONLY" = true ]; then
    echo -e "${YELLOW}[1/4] Python Linting${NC}"
    echo "───────────────────────────────────────"
    
    if [ "$AUTO_FIX" = true ]; then
        echo "Running ruff with auto-fix..."
        python3 -m ruff check PythonAPI/ --fix || true
        python3 -m ruff format PythonAPI/ || true
    else
        if python3 -m ruff check PythonAPI/ && python3 -m ruff format PythonAPI/ --check; then
            echo -e "${GREEN}✅ Python linting passed${NC}"
        else
            echo -e "${RED}❌ Python linting failed${NC}"
            ERRORS=$((ERRORS + 1))
        fi
    fi
    echo ""
fi

# C++ Linting
if [ "$CPP_ONLY" = true ]; then
    echo -e "${YELLOW}[2/4] C++ Linting${NC}"
    echo "───────────────────────────────────────"
    
    # Check if compilation database exists
    if [ ! -f "compile_commands.json" ] && [ ! -d "Build" ]; then
        echo -e "${YELLOW}⚠️  No compilation database found${NC}"
        echo "C++ linting requires a build. Skipping..."
        echo "Build with: cmake --preset=linux-base"
        echo -e "${GREEN}✅ C++ linting skipped (expected without build)${NC}"
    elif command -v /opt/homebrew/opt/llvm/bin/clang-tidy &> /dev/null || command -v clang-tidy &> /dev/null; then
        if [ "$AUTO_FIX" = true ]; then
            echo "Running clang-tidy with auto-fix..."
            ./lint-cpp.sh || true
        else
            if ./lint-cpp.sh; then
                echo -e "${GREEN}✅ C++ linting passed${NC}"
            else
                echo -e "${RED}❌ C++ linting failed${NC}"
                ERRORS=$((ERRORS + 1))
            fi
        fi
    else
        echo -e "${YELLOW}⚠️  clang-tidy not found, skipping C++ linting${NC}"
        echo "Install with: brew install llvm (macOS) or sudo apt-get install clang-tidy (Linux)"
    fi
    echo ""
fi

# Shell Linting
if [ "$SHELL_ONLY" = true ]; then
    echo -e "${YELLOW}[3/4] Shell Linting${NC}"
    echo "───────────────────────────────────────"
    
    if command -v shellcheck &> /dev/null; then
        if ./lint-shell.sh; then
            echo -e "${GREEN}✅ Shell linting passed${NC}"
        else
            echo -e "${RED}❌ Shell linting failed${NC}"
            ERRORS=$((ERRORS + 1))
        fi
    else
        echo -e "${YELLOW}⚠️  shellcheck not found, skipping shell linting${NC}"
        echo "Install with: brew install shellcheck (macOS) or sudo apt-get install shellcheck (Linux)"
    fi
    echo ""
fi

# YAML Linting
if [ "$YAML_ONLY" = true ]; then
    echo -e "${YELLOW}[4/4] YAML Linting${NC}"
    echo "───────────────────────────────────────"
    
    if command -v python3 &> /dev/null && python3 -c "import yamllint" 2>/dev/null; then
        YAML_ERRORS=$(python3 -m yamllint -c .yamllint . 2>&1 | grep -c "error:" || true)
        YAML_WARNINGS=$(python3 -m yamllint -c .yamllint . 2>&1 | grep -c "warning:" || true)
        
        if [ "$YAML_ERRORS" -eq 0 ]; then
            echo -e "${GREEN}✅ YAML linting passed${NC} ($YAML_WARNINGS warnings)"
        else
            echo -e "${RED}❌ YAML linting failed${NC} ($YAML_ERRORS errors, $YAML_WARNINGS warnings)"
            python3 -m yamllint -c .yamllint . 2>&1 | grep "error:" | head -10
            ERRORS=$((ERRORS + 1))
        fi
    else
        echo -e "${YELLOW}⚠️  yamllint not found, skipping YAML linting${NC}"
        echo "Install with: pip3 install yamllint"
    fi
    echo ""
fi

# Summary
echo "═══════════════════════════════════════════"
if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}✅ All linting checks passed!${NC}"
    exit 0
else
    echo -e "${RED}❌ Linting failed with $ERRORS error(s)${NC}"
    exit 1
fi
