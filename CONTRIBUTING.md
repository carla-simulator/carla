# Contributing to CARLA

## Code Quality

### Linting

Before submitting code, ensure all linters pass:

```bash
# Run all linters
./lint-all.sh

# Individual linters
python3 -m ruff check PythonAPI/
python3 -m ruff format PythonAPI/ --check
./lint-shell.sh
python3 -m yamllint -c .yamllint .
```

### Pre-commit Hook

Install the pre-commit hook to automatically run linters:

```bash
chmod +x .git/hooks/pre-commit
```

## Commit Messages

Follow conventional commits:
- `chore:` for infrastructure
- `refactor:` for code improvements
- `fix:` for bug fixes
