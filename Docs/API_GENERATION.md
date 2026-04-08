# API Documentation Generation

## Generate Python API Docs

```bash
# Install documentation dependencies
pip install pydoc-markdown mkdocs

# Generate documentation
pydoc-markdown

# Serve locally
mkdocs serve
```

## Documentation Structure

```
Docs/
├── API/              # Generated API docs
├── Architecture/     # Architecture docs
├── Guides/           # User guides
└── Reference/        # API reference
```

## Continuous Documentation

Documentation is automatically generated on each release via GitHub Actions.
