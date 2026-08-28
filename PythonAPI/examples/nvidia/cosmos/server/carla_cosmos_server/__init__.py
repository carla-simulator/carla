"""carla-cosmos server: HTTP/JSON job API in front of Cosmos worker processes.

Layout: :mod:`config` (env settings) -> :mod:`launcher` (profile, workers,
uvicorn) -> :mod:`app` (FastAPI: :mod:`auth`, :mod:`blobs`, :mod:`jobs`,
:mod:`models`, :mod:`metrics`, :mod:`ui`) -> :mod:`scheduler` (queues,
dispatch to :mod:`workers_rpc` handles) -> :mod:`store` (SQLite + files
under ``/state``); :mod:`gc` expires old blobs and jobs.
"""

__version__ = "0.2.0"
