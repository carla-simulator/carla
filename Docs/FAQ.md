# CARLA Frequently Asked Questions

## General

### What is CARLA?
CARLA is an open-source simulator for autonomous driving research.

### What platforms are supported?
- Ubuntu 22.04+
- Windows 11
- Docker containers

## Development

### How do I run linters?
```bash
./lint-all.sh
```

### How do I install pre-commit hook?
```bash
chmod +x .git/hooks/pre-commit
```

### How do I run tests?
```bash
python3 -m pytest PythonAPI/test -v
```

## Usage

### How do I spawn a vehicle?
See API examples in `Docs/Architecture/API_EXAMPLES.md`

### How do I attach sensors?
See sensor examples in PythonAPI/examples/

## Support

For more help, check:
- Documentation in Docs/
- GitHub issues
- Discord community
