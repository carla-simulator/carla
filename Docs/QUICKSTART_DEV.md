# Development Quick Start

## Setup (5 minutes)

```bash
# Clone repository
git clone https://github.com/carla-simulator/carla.git
cd carla

# Install dependencies
pip install -r requirements.txt

# Install pre-commit hook
chmod +x .git/hooks/pre-commit
```

## Daily Development

```bash
# Run linters before commit
./lint-all.sh

# Run tests
make test

# Run specific test
python3 -m pytest PythonAPI/test/unit/test_client.py -v
```

## Docker Development

```bash
# Build container
make docker-build

# Run tests in container
make docker-run

# Interactive development
docker-compose run carla-dev
```

## Common Tasks

See `Makefile` for all available commands:
```bash
make help
```
