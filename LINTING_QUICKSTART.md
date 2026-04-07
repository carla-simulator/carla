# CARLA Linting - Quick Start

## One Command

```bash
./lint-all.sh
```

## Install

```bash
pip install ruff yamllint && brew install llvm clang-format shellcheck
```

## Run Linters

```bash
# All linters
./lint-all.sh

# With auto-fix
./lint-all.sh --fix

# Individual
python3 -m ruff check PythonAPI/ --fix
./lint-shell.sh
python3 -m yamllint -c .yamllint .
./lint-cpp.sh  # requires build
```

## Status

✅ Python (107 files)  
✅ Shell (17 scripts)  
✅ YAML (17 files)  
⚙️ C++ (1044 files, requires build)

## Pre-commit

```bash
cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
set -e
python3 -m ruff check PythonAPI/ --fix
python3 -m ruff format PythonAPI/
./lint-shell.sh || true
EOF
chmod +x .git/hooks/pre-commit
```

## Files

- `pyproject.toml` - Python config
- `.clang-tidy` - C++ config
- `.shellcheckrc` - Shell config
- `.yamllint` - YAML config
- `lint-all.sh` - All-in-one script

## Help

```bash
./lint-all.sh --help
```

---

**Full guide**: `Docs/LINTING.md`
