# CARLA Troubleshooting Guide

## Common Issues

### Linting Errors

**Issue**: Pre-commit hook fails
```bash
# Solution: Run linters manually first
./lint-all.sh
```

### Python Import Errors

**Issue**: Module not found
```bash
# Solution: Set PYTHONPATH
export PYTHONPATH=/path/to/PythonAPI:$PYTHONPATH
```

### C++ Build Issues

**Issue**: Compilation database not found
```bash
# Solution: Build with CMake
cmake --preset=linux-base
cmake --build Build
```

### Docker Issues

**Issue**: Permission denied
```bash
# Solution: Run with sudo or add user to docker group
sudo usermod -aG docker $USER
```

## Getting Help

- Check existing issues on GitHub
- Read documentation in Docs/
- Join Discord community
