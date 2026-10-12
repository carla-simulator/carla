"""Shared fixtures.  Unit tests need no CARLA server; tests marked
``integration`` run only when ``CARLA_COSMOS_TEST_PORT`` is set."""

import os
import sys

import pytest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))


def pytest_collection_modifyitems(config, items):
    if os.environ.get("CARLA_COSMOS_TEST_PORT"):
        return
    skip = pytest.mark.skip(reason="CARLA_COSMOS_TEST_PORT not set")
    for item in items:
        if "integration" in item.keywords:
            item.add_marker(skip)


@pytest.fixture(scope="session")
def carla_client():
    """Client connected to the test server (integration tests only)."""
    import carla

    port = int(os.environ["CARLA_COSMOS_TEST_PORT"])
    client = carla.Client("127.0.0.1", port)
    client.set_timeout(60.0)
    return client
