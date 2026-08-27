"""Shared plumbing for the ``api_*.py`` demos (Python API, no CLI subprocess).

Every demo connects the same way the CLI does — ``--endpoint``/``--token``,
else ``COSMOS_URL`` / ``COSMOS_TOKEN`` — fails fast with an actionable message
when the node is unreachable or the backend is not loaded, and stores whatever
comes back through :class:`carla_cosmos.ResultStore`.
"""

from __future__ import annotations

import logging
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "client"))

from carla_cosmos import Clip, CosmosClient  # noqa: E402
from carla_cosmos.client import CosmosError  # noqa: E402
from carla_cosmos.results import StoredJob, default_results_root  # noqa: E402


def setup_logging(name: str, verbose: bool = False) -> logging.Logger:
    logging.basicConfig(level=logging.DEBUG if verbose else logging.INFO,
                        format="%(asctime)s %(levelname)-7s %(name)s: %(message)s")
    return logging.getLogger(name)


def connect(endpoint: str | None = None, token: str | None = None, backend: str | None = None) -> CosmosClient:
    """Client for the Cosmos node; ``SystemExit`` with a clear message if it cannot be used."""
    cosmos = CosmosClient(endpoint, token=token)
    try:
        live = cosmos.health()
    except CosmosError as exc:
        raise SystemExit(f"cannot reach the Cosmos node at {cosmos.url}: {exc}\n"
                         f"  start it (carla-cosmos serve) or point COSMOS_URL / --endpoint at the node, "
                         f"and set COSMOS_TOKEN / --token") from None
    ready, body = cosmos.ready()
    if not ready:
        stuck = ", ".join(f"{w['name']}={w['state']}" for w in body.get("workers", []))
        raise SystemExit(f"{cosmos.url} answers (version {live.get('version')}) but is not ready yet: "
                         f"{stuck or body}\n  wait for the workers to load, then run this again")
    if backend:
        models = cosmos.models()
        if backend not in models:
            raise SystemExit(f"{cosmos.url} does not serve '{backend}' (it has: {', '.join(sorted(models))})")
        if not models[backend].available:
            raise SystemExit(f"'{backend}' is not loaded on {cosmos.url} (profile has: "
                             f"{', '.join(sorted(m for m, i in models.items() if i.available)) or 'nothing'})")
    return cosmos


def load_clip(path: str) -> Clip:
    """``Clip.load`` with a message instead of a traceback when the directory is not a clip."""
    try:
        return Clip.load(path)
    except (OSError, ValueError) as exc:
        raise SystemExit(f"{path} is not a captured clip directory (needs manifest.json): {exc}\n"
                         f"  capture one with api_capture_to_job.py, single_view_live.py or "
                         f"carla-cosmos synthetic-clip --out ./clips") from None


def progress_logger(log: logging.Logger, tag: str = ""):
    """``on_progress`` callback that logs each distinct state/percent/message."""
    last = [""]

    def on_progress(info) -> None:
        line = f"{info.status} {info.progress * 100:5.1f}% {info.message}"
        if line != last[0]:
            log.info("%s%s", f"[{tag}] " if tag else "", line)
            last[0] = line

    return on_progress


def wait_and_store(job, results: str | None, log: logging.Logger, tag: str = "") -> StoredJob:
    """Wait for the job and keep every returned file under ``<results>/<clip_id>/<job_id>/``."""
    job.wait(poll=3.0, on_progress=progress_logger(log, tag))
    stored = job.download(results)
    log.info("stored %d file(s), %.1f MB in %s", len(stored.files), stored.bytes / 1e6, stored.directory)
    for f in stored.files:
        log.info("  %-42s %8.1f kB  %s", f.name, f.size / 1e3, f.kind)
    left = stored.expires_in_hours()
    if left is not None:
        log.info("the server keeps its copy for another %.0f h; this local copy does not expire", left)
    return stored


def results_root(results: str | None) -> Path:
    """Where results will land (``--results``, ``$COSMOS_RESULTS``, ``./cosmos-results``)."""
    return default_results_root(results)
