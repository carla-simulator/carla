"""carla-agentic-tools: a standalone MCP server exposing the CARLA skill registry."""

# Keep in step with [project].version in pyproject.toml. tests/test_version.py
# fails if the two drift -- which is how this string sat at 0.1.0 while the
# project shipped 0.3.0. Do not read it from importlib.metadata instead: that
# reports whatever is INSTALLED, which is wrong (and confusing) when running
# from a checkout with an older wheel in site-packages.
__version__ = "0.6.0"

__all__ = ["__version__"]
