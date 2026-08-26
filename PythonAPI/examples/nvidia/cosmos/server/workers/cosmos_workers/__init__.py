"""Worker adapters for the carla-cosmos server.

Each worker is a separate process, started by the server launcher inside the
venv that holds its model stack, and talks to the API process over a unix
socket (:mod:`cosmos_workers.common.protocol`).  ``cosmos_workers.mock`` is a
CPU-only worker used by the tests and the smoke test.
"""

__version__ = "0.2.0"
