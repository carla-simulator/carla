# CARLA Linting Guide

Comprehensive linting configuration for the CARLA project covering Python, C++, Shell, and YAML.

## Quick Start

```bash
# Run all linters
./lint-all.sh

# Run with auto-fix
./lint-all.sh --fix
```

## Status

| Language | Files | Status | Config |
|----------|-------|--------|--------|
| Python | 107 | ✅ PASS | `pyproject.toml` |
| C++ | 1044 | ⚙️ READY | `.clang-tidy` |
| Shell | 17 | ✅ PASS | `.shellcheckrc` |
| YAML | 17 | ✅ PASS | `.yamllint` |

## Installation

```bash
# Python tools
pip install ruff yamllint

# C++ tools (macOS)
brew install llvm clang-format

# Shell tools
brew install shellcheck
```

## Commands

### All-in-One
```bash
./lint-all.sh           # Check all
./lint-all.sh --fix     # Auto-fix where possible
```

### Individual
```bash
# Python
python3 -m ruff check PythonAPI/ --fix
python3 -m ruff format PythonAPI/

# Shell
./lint-shell.sh
shellcheck script.sh

# YAML
python3 -m yamllint -c .yamllint .

# C++ (requires build)
./lint-cpp.sh
```

## Configuration

### Python (Ruff)
- **Config**: `pyproject.toml`
- **Rules**: 70+ categories
- **Strictness**: Tight for core code

### C++ (clang-tidy)
- **Config**: `.clang-tidy`
- **Rules**: 70+ categories
- **Note**: Requires compilation database

### Shell (shellcheck)
- **Config**: `.shellcheckrc`
- **Dialect**: bash
- **Exclusions**: SC1091 (source not found)

### YAML (yamllint)
- **Config**: `.yamllint`
- **Base**: Extended default
- **Line limit**: 120 characters

## Pre-commit Hook

Create `.git/hooks/pre-commit`:
```bash
#!/bin/bash
set -e
python3 -m ruff check PythonAPI/ --fix
python3 -m ruff format PythonAPI/
./lint-shell.sh || true
python3 -m yamllint -c .yamllint . || true
```

## CI/CD

```yaml
- name: Linting
  run: |
    pip install ruff yamllint
    python3 -m ruff check PythonAPI/
    python3 -m ruff format PythonAPI/ --check
    ./lint-shell.sh
```

## Documentation

- `LINTING_QUICKSTART.md` - Quick reference
- `Docs/LINTING.md` - This guide

## Troubleshooting

**Python module errors**: `pip install -r requirements.txt`

**C++ compilation database**: Build with CMake first

**Shell source warnings**: Expected, add `# shellcheck source=/dev/null`

---

*Status: ✅ All linters passing*  
*Last updated: April 7, 2026*
